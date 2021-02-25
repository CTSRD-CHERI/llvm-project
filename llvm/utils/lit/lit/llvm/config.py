import os
import platform
import re
import subprocess
import sys

import lit.util
from lit.llvm.subst import FindTool
from lit.llvm.subst import ToolSubst


class LLVMConfig(object):

    def __init__(self, lit_config, config):
        self.lit_config = lit_config
        self.config = config

        features = config.available_features

        self.use_lit_shell = False
        # Tweak PATH for Win32 to decide to use bash.exe or not.
        if sys.platform == 'win32':
            # For tests that require Windows to run.
            features.add('system-windows')

            # Seek sane tools in directories and set to $PATH.
            path = self.lit_config.getToolsPath(config.lit_tools_dir,
                                                config.environment['PATH'],
                                                ['cmp.exe', 'grep.exe', 'sed.exe'])
            if path is not None:
                self.with_environment('PATH', path, append_path=True)
            # Many tools behave strangely if these environment variables aren't set.
            self.with_system_environment(['SystemDrive', 'SystemRoot', 'TEMP', 'TMP'])
            self.use_lit_shell = True

        # Choose between lit's internal shell pipeline runner and a real shell.  If
        # LIT_USE_INTERNAL_SHELL is in the environment, we use that as an override.
        lit_shell_env = os.environ.get('LIT_USE_INTERNAL_SHELL')
        if lit_shell_env:
            self.use_lit_shell = lit.util.pythonize_bool(lit_shell_env)

        if not self.use_lit_shell:
            features.add('shell')

        # Running on Darwin OS
        if platform.system() == 'Darwin':
            # FIXME: lld uses the first, other projects use the second.
            # We should standardize on the former.
            features.add('system-linker-mach-o')
            features.add('system-darwin')
        elif platform.system() == 'Windows':
            # For tests that require Windows to run.
            features.add('system-windows')
        elif platform.system() == 'Linux':
            features.add('system-linux')
        elif platform.system() in ['FreeBSD']:
            features.add('system-freebsd')
        elif platform.system() == 'NetBSD':
            features.add('system-netbsd')
        elif platform.system() == 'AIX':
            features.add('system-aix')

        # Native compilation: host arch == default triple arch
        # Both of these values should probably be in every site config (e.g. as
        # part of the standard header.  But currently they aren't)
        host_triple = getattr(config, 'host_triple', None)
        target_triple = getattr(config, 'target_triple', None)
        if host_triple and host_triple == target_triple:
            features.add('native')

        # Sanitizers.
        sanitizers = getattr(config, 'llvm_use_sanitizer', '')
        sanitizers = frozenset(x.lower() for x in sanitizers.split(';'))
        if 'address' in sanitizers:
            features.add('asan')
        if 'memory' in sanitizers or 'memorywithorigins' in sanitizers:
            features.add('msan')
        if 'undefined' in sanitizers:
            features.add('ubsan')

        have_zlib = getattr(config, 'have_zlib', None)
        if have_zlib:
            features.add('zlib')

        # Check if we should run long running tests.
        long_tests = lit_config.params.get('run_long_tests', None)
        if lit.util.pythonize_bool(long_tests):
            features.add('long_tests')

        if target_triple:
            if re.match(r'^x86_64.*-apple', target_triple):
                features.add('x86_64-apple')
                host_cxx = getattr(config, 'host_cxx', None)
                if 'address' in sanitizers and self.get_clang_has_lsan(host_cxx, target_triple):
                    self.with_environment(
                        'ASAN_OPTIONS', 'detect_leaks=1', append_path=True)
            if re.match(r'^x86_64.*-linux', target_triple):
                features.add('x86_64-linux')
            if re.match(r'^i.86.*', target_triple):
                features.add('target-x86')
            elif re.match(r'^x86_64.*', target_triple):
                features.add('target-x86_64')
            elif re.match(r'^aarch64.*', target_triple):
                features.add('target-aarch64')
            elif re.match(r'^arm.*', target_triple):
                features.add('target-arm')

        use_gmalloc = lit_config.params.get('use_gmalloc', None)
        if lit.util.pythonize_bool(use_gmalloc):
            # Allow use of an explicit path for gmalloc library.
            # Will default to '/usr/lib/libgmalloc.dylib' if not set.
            gmalloc_path_str = lit_config.params.get('gmalloc_path',
                                                     '/usr/lib/libgmalloc.dylib')
            if gmalloc_path_str is not None:
                self.with_environment(
                    'DYLD_INSERT_LIBRARIES', gmalloc_path_str)

    def with_environment(self, variable, value, append_path=False):
        if append_path:
            # For paths, we should be able to take a list of them and process all
            # of them.
            paths_to_add = value
            if lit.util.is_string(paths_to_add):
                paths_to_add = [paths_to_add]

            def norm(x):
                return os.path.normcase(os.path.normpath(x))

            current_paths = self.config.environment.get(variable, None)
            if current_paths:
                current_paths = current_paths.split(os.path.pathsep)
                paths = [norm(p) for p in current_paths]
            else:
                paths = []

            # If we are passed a list [a b c], then iterating this list forwards
            # and adding each to the beginning would result in b c a.  So we
            # need to iterate in reverse to end up with the original ordering.
            for p in reversed(paths_to_add):
                # Move it to the front if it already exists, otherwise insert it at the
                # beginning.
                p = norm(p)
                try:
                    paths.remove(p)
                except ValueError:
                    pass
                paths = [p] + paths
            value = os.pathsep.join(paths)
        self.config.environment[variable] = value

    def with_system_environment(self, variables, append_path=False):
        if lit.util.is_string(variables):
            variables = [variables]
        for v in variables:
            value = os.environ.get(v)
            if value:
                self.with_environment(v, value, append_path)

    def clear_environment(self, variables):
        for name in variables:
            if name in self.config.environment:
                del self.config.environment[name]

    def get_process_output(self, command):
        try:
            cmd = subprocess.Popen(
                command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env=self.config.environment)
            stdout, stderr = cmd.communicate()
            stdout = lit.util.to_string(stdout)
            stderr = lit.util.to_string(stderr)
            return (stdout, stderr)
        except OSError:
            self.lit_config.fatal('Could not run process %s' % command)

    def feature_config(self, features):
        # Ask llvm-config about the specified feature.
        arguments = [x for (x, _) in features]
        config_path = os.path.join(self.config.llvm_tools_dir, 'llvm-config')

        output, _ = self.get_process_output([config_path] + arguments)
        lines = output.split('\n')

        for (feature_line, (_, patterns)) in zip(lines, features):
            # We should have either a callable or a dictionary.  If it's a
            # dictionary, grep each key against the output and use the value if
            # it matches.  If it's a callable, it does the entire translation.
            if callable(patterns):
                features_to_add = patterns(feature_line)
                self.config.available_features.update(features_to_add)
            else:
                for (re_pattern, feature) in patterns.items():
                    if re.search(re_pattern, feature_line):
                        self.config.available_features.add(feature)

    # Note that when substituting %clang_cc1 also fill in the include directory of
    # the builtin headers. Those are part of even a freestanding environment, but
    # Clang relies on the driver to locate them.
    def get_clang_builtin_include_dir(self, clang):
        # FIXME: Rather than just getting the version, we should have clang print
        # out its resource dir here in an easy to scrape form.
        clang_dir, _ = self.get_process_output(
            [clang, '-print-file-name=include'])

        if not clang_dir:
            self.lit_config.fatal(
                "Couldn't find the include dir for Clang ('%s')" % clang)

        clang_dir = clang_dir.strip()
        if sys.platform in ['win32'] and not self.use_lit_shell:
            # Don't pass dosish path separator to msys bash.exe.
            clang_dir = clang_dir.replace('\\', '/')
        # Ensure the result is an ascii string, across Python2.5+ - Python3.
        return clang_dir

    # On macOS, LSan is only supported on clang versions 5 and higher
    def get_clang_has_lsan(self, clang, triple):
        if not clang:
            self.lit_config.warning(
                'config.host_cxx is unset but test suite is configured to use sanitizers.')
            return False

        clang_binary = clang.split()[0]
        version_string, _ = self.get_process_output(
            [clang_binary, '--version'])
        if not 'clang' in version_string:
            self.lit_config.warning(
                "compiler '%s' does not appear to be clang, " % clang_binary +
                'but test suite is configured to use sanitizers.')
            return False

        if re.match(r'.*-linux', triple):
            return True

        if re.match(r'^x86_64.*-apple', triple):
            version_regex = re.search(r'version ([0-9]+)\.([0-9]+).([0-9]+)', version_string)
            major_version_number = int(version_regex.group(1))
            minor_version_number = int(version_regex.group(2))
            patch_version_number = int(version_regex.group(3))
            if ('Apple LLVM' in version_string) or ('Apple clang' in version_string):
                # Apple clang doesn't yet support LSan
                return False
            else:
                return major_version_number >= 5

        return False

    def make_itanium_abi_triple(self, triple):
        m = re.match(r'(\w+)-(\w+)-(\w+)', triple)
        if not m:
            self.lit_config.fatal(
                "Could not turn '%s' into Itanium ABI triple" % triple)
        if m.group(3).lower() != 'windows':
            # All non-windows triples use the Itanium ABI.
            return triple
        return m.group(1) + '-' + m.group(2) + '-' + m.group(3) + '-gnu'

    def make_msabi_triple(self, triple):
        m = re.match(r'(\w+)-(\w+)-(\w+)', triple)
        if not m:
            self.lit_config.fatal(
                "Could not turn '%s' into MS ABI triple" % triple)
        isa = m.group(1).lower()
        vendor = m.group(2).lower()
        os = m.group(3).lower()
        if os == 'windows' and re.match(r'.*-msvc$', triple):
            # If the OS is windows and environment is msvc, we're done.
            return triple
        if isa.startswith('x86') or isa == 'amd64' or re.match(r'i\d86', isa):
            # For x86 ISAs, adjust the OS.
            return isa + '-' + vendor + '-windows-msvc'
        # -msvc is not supported for non-x86 targets; use a default.
        return 'i686-pc-windows-msvc'

    def add_tool_substitutions(self, tools, search_dirs=None):
        if not search_dirs:
            search_dirs = [self.config.llvm_tools_dir]

        if lit.util.is_string(search_dirs):
            search_dirs = [search_dirs]

        tools = [x if isinstance(x, ToolSubst) else ToolSubst(x)
                 for x in tools]

        search_dirs = os.pathsep.join(search_dirs)
        substitutions = []

        for tool in tools:
            match = tool.resolve(self, search_dirs)

            # Either no match occurred, or there was an unresolved match that
            # is ignored.
            if not match:
                continue

            subst_key, tool_pipe, command = match

            # An unresolved match occurred that can't be ignored.  Fail without
            # adding any of the previously-discovered substitutions.
            if not command:
                return False

            substitutions.append((subst_key, tool_pipe + command))

        self.config.substitutions.extend(substitutions)
        return True

    def use_default_substitutions(self):
        tool_patterns = [
            ToolSubst('FileCheck', unresolved='fatal'),
            # Handle these specially as they are strings searched for during testing.
            ToolSubst(r'\| \bcount\b', command=FindTool(
                'count'), verbatim=True, unresolved='fatal'),
            ToolSubst(r'\| \bnot\b', command=FindTool('not'), verbatim=True, unresolved='fatal')]

        default_cheri_size = self.lit_config.params['CHERI_CAP_SIZE']
        tool_patterns.append(ToolSubst('%cheri128_FileCheck', FindTool('FileCheck'),
                                       extra_args=['-D\\#CAP_SIZE=16']))
        tool_patterns.append(ToolSubst('%cheri64_FileCheck', FindTool('FileCheck'),
                                       extra_args=['-D\\#CAP_SIZE=8']))
        tool_patterns.append(ToolSubst('%cheri_FileCheck', FindTool('FileCheck'),
                                       extra_args=['-D\\#CAP_SIZE=' + default_cheri_size]))
        if not self.lit_config.quiet:
            self.lit_config.note('Running tests for CHERI_CAP_SIZE=' + default_cheri_size)

        self.config.substitutions.append(('%python', '"%s"' % (sys.executable)))

        self.add_tool_substitutions(
            tool_patterns, [self.config.llvm_tools_dir])

    def _add_cheri_tool_substitution(self, tool):
        assert tool in ('llc', 'opt', 'llvm-mc'), 'Invalid tool: ' + tool
        default_cheri_size = self.lit_config.params['CHERI_CAP_SIZE']
        if tool == 'llvm-mc':
            triple_opt = '-triple'
            purecap_args = ['-target-abi', 'purecap', '-position-independent']
        else:
            triple_opt = '-mtriple'
            purecap_args = ['-target-abi', 'purecap', '-relocation-model', 'pic']
        extra_args = []
        if tool == "llc":  # TODO: add this to clang as well?
            extra_args = ["-verify-machineinstrs"]
        cheri128_args = [triple_opt + '=mips64-unknown-freebsd', '-mcpu=cheri128', '-mattr=+cheri128'] + extra_args
        riscv32_cheri_args = [triple_opt + '=riscv32-unknown-freebsd', '-mattr=+xcheri'] + extra_args
        riscv64_cheri_args = [triple_opt + '=riscv64-unknown-freebsd', '-mattr=+xcheri'] + extra_args
        riscv32_cheri_purecap_args = ['-target-abi', 'il32pc64', '-mattr=+cap-mode'] + riscv32_cheri_args
        riscv64_cheri_purecap_args = ['-target-abi', 'l64pc128', '-mattr=+cap-mode'] + riscv64_cheri_args

        assert default_cheri_size == '16', "cap size=" + default_cheri_size + "no longer supported"
        default_args = cheri128_args
        self.config.substitutions.append(('%cheri_cap_bytes', default_cheri_size))
        tool_patterns = [
            ToolSubst('%cheri_' + tool, FindTool(tool), extra_args=default_args),
            ToolSubst('%cheri128_' + tool, FindTool(tool), extra_args=cheri128_args),
            ToolSubst('%cheri_purecap_' + tool, FindTool(tool),
                      extra_args=default_args + purecap_args),
            ToolSubst('%cheri128_purecap_' + tool, FindTool(tool),
                      extra_args=cheri128_args + purecap_args),
            ToolSubst('%riscv32_cheri_' + tool, FindTool(tool), extra_args=riscv32_cheri_args),
            ToolSubst('%riscv64_cheri_' + tool, FindTool(tool), extra_args=riscv64_cheri_args),
            ToolSubst('%riscv32_cheri_purecap_' + tool, FindTool(tool), extra_args=riscv32_cheri_purecap_args),
            ToolSubst('%riscv64_cheri_purecap_' + tool, FindTool(tool), extra_args=riscv64_cheri_purecap_args),
        ]
        self.add_tool_substitutions(tool_patterns, [self.config.llvm_tools_dir])

    def add_cheri_tool_substitutions(self, tools):
        for tool in tools:
            self._add_cheri_tool_substitution(tool)

    def use_llvm_tool(self, name, search_env=None, required=False, quiet=False):
        """Find the executable program 'name', optionally using the specified
        environment variable as an override before searching the
        configuration's PATH."""
        # If the override is specified in the environment, use it without
        # validation.
        if search_env:
            tool = self.config.environment.get(search_env)
            if tool:
                return tool

        # Otherwise look in the path.
        tool = lit.util.which(name, self.config.environment['PATH'])

        if required and not tool:
            message = "couldn't find '{}' program".format(name)
            if search_env:
                message = message + \
                    ', try setting {} in your environment'.format(search_env)
            self.lit_config.fatal(message)

        if tool:
            tool = os.path.normpath(tool)
            if not self.lit_config.quiet and not quiet:
                self.lit_config.note('using {}: {}'.format(name, tool))
        return tool

    def use_clang(self, additional_tool_dirs=[], additional_flags=[], required=True):
        """Configure the test suite to be able to invoke clang.

        Sets up some environment variables important to clang, locates a
        just-built or installed clang, and add a set of standard
        substitutions useful to any test suite that makes use of clang.

        """
        # Clear some environment variables that might affect Clang.
        #
        # This first set of vars are read by Clang, but shouldn't affect tests
        # that aren't specifically looking for these features, or are required
        # simply to run the tests at all.
        #
        # FIXME: Should we have a tool that enforces this?

        # safe_env_vars = ('TMPDIR', 'TEMP', 'TMP', 'USERPROFILE', 'PWD',
        #                  'MACOSX_DEPLOYMENT_TARGET', 'IPHONEOS_DEPLOYMENT_TARGET',
        #                  'VCINSTALLDIR', 'VC100COMNTOOLS', 'VC90COMNTOOLS',
        #                  'VC80COMNTOOLS')
        possibly_dangerous_env_vars = ['COMPILER_PATH', 'RC_DEBUG_OPTIONS',
                                       'CINDEXTEST_PREAMBLE_FILE', 'LIBRARY_PATH',
                                       'CPATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH',
                                       'OBJC_INCLUDE_PATH', 'OBJCPLUS_INCLUDE_PATH',
                                       'LIBCLANG_TIMING', 'LIBCLANG_OBJTRACKING',
                                       'LIBCLANG_LOGGING', 'LIBCLANG_BGPRIO_INDEX',
                                       'LIBCLANG_BGPRIO_EDIT', 'LIBCLANG_NOTHREADS',
                                       'LIBCLANG_RESOURCE_USAGE',
                                       'LIBCLANG_CODE_COMPLETION_LOGGING']
        # Clang/Win32 may refer to %INCLUDE%. vsvarsall.bat sets it.
        if platform.system() != 'Windows':
            possibly_dangerous_env_vars.append('INCLUDE')

        self.clear_environment(possibly_dangerous_env_vars)

        # Tweak the PATH to include the tools dir and the scripts dir.
        # Put Clang first to avoid LLVM from overriding out-of-tree clang builds.
        exe_dir_props = [self.config.name.lower() + '_tools_dir', 'clang_tools_dir', 'llvm_tools_dir']
        paths = [getattr(self.config, pp) for pp in exe_dir_props
                 if getattr(self.config, pp, None)]
        paths = additional_tool_dirs + paths
        self.with_environment('PATH', paths, append_path=True)

        lib_dir_props = [self.config.name.lower() + '_libs_dir', 'clang_libs_dir', 'llvm_shlib_dir', 'llvm_libs_dir']
        paths = [getattr(self.config, pp) for pp in lib_dir_props
                 if getattr(self.config, pp, None)]

        self.with_environment('LD_LIBRARY_PATH', paths, append_path=True)

        # Discover the 'clang' and 'clangcc' to use.

        self.config.clang = self.use_llvm_tool(
            'clang', search_env='CLANG', required=required)
        if not self.config.clang:
            return

        shl = getattr(self.config, 'llvm_shlib_dir', None)
        pext = getattr(self.config, 'llvm_plugin_ext', None)
        if shl:
            self.config.substitutions.append(('%llvmshlibdir', shl))
        if pext:
            self.config.substitutions.append(('%pluginext', pext))

        builtin_include_dir = self.get_clang_builtin_include_dir(self.config.clang)
        clang_cc1_args = ['-cc1', '-internal-isystem', builtin_include_dir, '-nostdsysteminc']
        cheri128_cc1_args = clang_cc1_args + ['-triple', 'mips64-unknown-freebsd',
                '-target-cpu', 'cheri128', '-cheri-size', '128', '-mllvm', '-verify-machineinstrs']
        riscv32_cheri_cc1_args = clang_cc1_args + ['-triple', 'riscv32-unknown-freebsd',
                '-target-feature', '+xcheri', '-mllvm', '-verify-machineinstrs']
        riscv64_cheri_cc1_args = clang_cc1_args + ['-triple', 'riscv64-unknown-freebsd',
                '-target-feature', '+xcheri', '-mllvm', '-verify-machineinstrs']

        default_cheri_size = self.lit_config.params['CHERI_CAP_SIZE']
        assert default_cheri_size == '16', "cap size=" + default_cheri_size + "no longer supported"
        cheri_cc1_args = cheri128_cc1_args
        default_cheri_cpu = 'cheri128'
        cheri_clang_args = ['-target', 'mips64-unknown-freebsd', '-nostdinc',
                            '-mcpu=' + default_cheri_cpu, '-msoft-float']
        riscv32_cheri_clang_args = ['-target', 'riscv32-unknown-freebsd', '-nostdinc', '-march=rv32imafdcxcheri']
        riscv64_cheri_clang_args = ['-target', 'riscv64-unknown-freebsd', '-nostdinc', '-march=rv64imafdcxcheri']

        tool_substitutions = [
            # CHERI substitutions (order is important due to repeated substitutions!)
            ToolSubst('%cheri_purecap_cc1',    command='%cheri_cc1',    extra_args=['-target-abi', 'purecap']+additional_flags),
            ToolSubst('%cheri128_purecap_cc1', command='%cheri128_cc1', extra_args=['-target-abi', 'purecap']+additional_flags),
            ToolSubst('%cheri_cc1',    command=self.config.clang, extra_args=cheri_cc1_args+additional_flags),
            ToolSubst('%cheri128_cc1', command=self.config.clang, extra_args=cheri128_cc1_args+additional_flags),
            ToolSubst('%cheri_clang', command=self.config.clang, extra_args=cheri_clang_args+additional_flags),
            ToolSubst('%cheri_purecap_clang', command=self.config.clang,
                      extra_args=cheri_clang_args + ['-mabi=purecap']+additional_flags),
            ToolSubst('%riscv32_cheri_purecap_cc1', command='%riscv32_cheri_cc1', extra_args=['-target-abi', 'il32pc64', '-target-feature', '+cap-mode']+additional_flags),
            ToolSubst('%riscv64_cheri_purecap_cc1', command='%riscv64_cheri_cc1', extra_args=['-target-abi', 'l64pc128', '-target-feature', '+cap-mode']+additional_flags),
            ToolSubst('%riscv32_cheri_purecap_clang', command='%riscv32_cheri_clang', extra_args=['-mabi=il32pc64']+additional_flags),
            ToolSubst('%riscv64_cheri_purecap_clang', command='%riscv64_cheri_clang', extra_args=['-mabi=l64pc128']+additional_flags),
            ToolSubst('%riscv32_cheri_cc1', command=self.config.clang, extra_args=riscv32_cheri_cc1_args+additional_flags),
            ToolSubst('%riscv64_cheri_cc1', command=self.config.clang, extra_args=riscv64_cheri_cc1_args+additional_flags),
            ToolSubst('%riscv32_cheri_clang', command=self.config.clang, extra_args=riscv32_cheri_clang_args+additional_flags),
            ToolSubst('%riscv64_cheri_clang', command=self.config.clang, extra_args=riscv64_cheri_clang_args+additional_flags),
            # For the tests in Driver that don't depend on the capability size
            ToolSubst('%plain_clang_cheri_triple_allowed', command=self.config.clang, extra_args=additional_flags),

            ToolSubst('%clang', command=self.config.clang, extra_args=additional_flags),
            ToolSubst('%clang_analyze_cc1', command='%clang_cc1', extra_args=['-analyze', '%analyze', '-setup-static-analyzer']+additional_flags),
            ToolSubst('%clang_cc1', command=self.config.clang, extra_args=clang_cc1_args+additional_flags),
            ToolSubst('%clang_cpp', command=self.config.clang, extra_args=['--driver-mode=cpp']+additional_flags),
            ToolSubst('%clang_cl', command=self.config.clang, extra_args=['--driver-mode=cl']+additional_flags),
            ToolSubst('%clangxx', command=self.config.clang, extra_args=['--driver-mode=g++']+additional_flags),
            ]
        self.add_tool_substitutions(tool_substitutions)

        self.config.substitutions.append(('%itanium_abi_triple',
                                          self.make_itanium_abi_triple(self.config.target_triple)))
        self.config.substitutions.append(('%ms_abi_triple',
                                          self.make_msabi_triple(self.config.target_triple)))
        self.config.substitutions.append(
            ('%resource_dir', builtin_include_dir))

        # The host triple might not be set, at least if we're compiling clang from
        # an already installed llvm.
        if self.config.host_triple and self.config.host_triple != '@LLVM_HOST_TRIPLE@':
            self.config.substitutions.append(('%target_itanium_abi_host_triple',
                                              '--target=%s' % self.make_itanium_abi_triple(self.config.host_triple)))
        else:
            self.config.substitutions.append(
                ('%target_itanium_abi_host_triple', ''))

        # FIXME: Find nicer way to prohibit this.
        self.config.substitutions.append(
            (' clang ', """\"*** Do not use 'clang' in tests, use '%clang'. ***\""""))
        self.config.substitutions.append(
            (' clang\+\+ ', """\"*** Do not use 'clang++' in tests, use '%clangxx'. ***\""""))
        self.config.substitutions.append(
            (' clang-cc ',
             """\"*** Do not use 'clang-cc' in tests, use '%clang_cc1'. ***\""""))
        self.config.substitutions.append(
            (' clang-cl ',
             """\"*** Do not use 'clang-cl' in tests, use '%clang_cl'. ***\""""))
        self.config.substitutions.append(
            (' clang -cc1 -analyze ',
             """\"*** Do not use 'clang -cc1 -analyze' in tests, use '%clang_analyze_cc1'. ***\""""))
        self.config.substitutions.append(
            (' clang -cc1 ',
             """\"*** Do not use 'clang -cc1' in tests, use '%clang_cc1'. ***\""""))
        self.config.substitutions.append(
            (' %clang-cc1 ',
             """\"*** invalid substitution, use '%clang_cc1'. ***\""""))
        self.config.substitutions.append(
            (' %clang-cpp ',
             """\"*** invalid substitution, use '%clang_cpp'. ***\""""))
        self.config.substitutions.append(
            (' %clang-cl ',
             """\"*** invalid substitution, use '%clang_cl'. ***\""""))

    def use_lld(self, additional_tool_dirs=[], required=True):
        """Configure the test suite to be able to invoke lld.

        Sets up some environment variables important to lld, locates a
        just-built or installed lld, and add a set of standard
        substitutions useful to any test suite that makes use of lld.

        """

        # Tweak the PATH to include the tools dir and the scripts dir.
        exe_dir_props = [self.config.name.lower() + '_tools_dir', 'lld_tools_dir', 'llvm_tools_dir']
        paths = [getattr(self.config, pp) for pp in exe_dir_props
                 if getattr(self.config, pp, None)]
        paths = additional_tool_dirs + paths
        self.with_environment('PATH', paths, append_path=True)

        lib_dir_props = [self.config.name.lower() + '_libs_dir', 'lld_libs_dir', 'llvm_libs_dir']
        paths = [getattr(self.config, pp) for pp in lib_dir_props
                 if getattr(self.config, pp, None)]

        self.with_environment('LD_LIBRARY_PATH', paths, append_path=True)

        # Discover the 'clang' and 'clangcc' to use.

        ld_lld = self.use_llvm_tool('ld.lld', required=required)
        lld_link = self.use_llvm_tool('lld-link', required=required)
        ld64_lld = self.use_llvm_tool('ld64.lld', required=required)
        wasm_ld = self.use_llvm_tool('wasm-ld', required=required)

        was_found = ld_lld and lld_link and ld64_lld and wasm_ld
        tool_substitutions = []
        if ld_lld:
            tool_substitutions.append(ToolSubst('ld\.lld', command=ld_lld))
        if lld_link:
            tool_substitutions.append(ToolSubst('lld-link', command=lld_link))
        if ld64_lld:
            tool_substitutions.append(ToolSubst('ld64\.lld', command=ld64_lld))
        if wasm_ld:
            tool_substitutions.append(ToolSubst('wasm-ld', command=wasm_ld))
        self.add_tool_substitutions(tool_substitutions)
        return was_found
