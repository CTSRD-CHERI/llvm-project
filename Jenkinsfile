@Library('ctsrd-jenkins-scripts') _

properties([disableConcurrentBuilds(),
            // compressBuildLog(), // Broken, see https://issues.jenkins-ci.org/browse/JENKINS-54680
            disableResume(),
            [$class: 'GithubProjectProperty', displayName: '', projectUrlStr: 'https://github.com/CTSRD-CHERI/llvm-project/'],
            [$class: 'CopyArtifactPermissionProperty', projectNames: '*'],
            [$class: 'JobPropertyImpl', throttle: [count: 2, durationName: 'hour', userBoost: true]],
            durabilityHint('PERFORMANCE_OPTIMIZED'),
            pipelineTriggers([githubPush()])
])

// global vars needed to update github status
llvmRepo = null
TEST_RELEASE_BUILD = false

def updateGithubStatus(String message) {
    setGitHubStatus(llvmRepo, [message: message])
}

def doGit(String url, String branch, String subdir, String referenceDirName) {
    String referenceDir = "/var/tmp/git-reference-repos/${referenceDirName}"
    def options = [ changelog: true, poll: true,
                    scm: [$class: 'GitSCM',
                          doGenerateSubmoduleConfigurations: false,
                          branches: [[name: "refs/heads/${branch}"]],
                          extensions: [/* to skip polling: [$class: 'IgnoreNotifyCommit'], */
                                       [$class: 'RelativeTargetDirectory', relativeTargetDir: subdir],
                                       [$class: 'CloneOption', noTags: false, reference: referenceDir, shallow: false, depth: 10, timeout: 60],
                                       // Uncomment for git problems debugging: /* Clean clone: */ [$class: 'WipeWorkspace'],
                                       // [$class: 'LocalBranch', localBranch: branch]
                          ],
                          userRemoteConfigs: [[url: url, credentialsId: 'ctsrd-jenkins-api-token-with-username']]
                    ]
    ]
    echo("Git options: ${options}")
    def result = checkout(options)
    dir(subdir) {
        sh """
set -xe
pwd
git branch
git status
git log -3
"""
    }
    return result
}

def runTests(String targetSuffix) {
    stage("Run tests (${targetSuffix})") {
        updateGithubStatus("Running check-${targetSuffix} tests...")
        sh """#!/usr/bin/env bash
set -xe

cd \${WORKSPACE}/llvm-project/Build
# run tests
if [ "$label" = "freebsd" ]; then
  # without this lit attempts to encode some things as ASCII and fails
  export LC_ALL=en_US.UTF-8
fi
rm -fv "\${WORKSPACE}/llvm-test-output.xml"
# Verbose filecheck failure output:
export FILECHECK_DUMP_INPUT_ON_FAILURE=1
ninja check-${targetSuffix} \${JFLAG} || echo "Some check-${targetSuffix} tests failed!"
mv -fv "\${WORKSPACE}/llvm-test-output.xml" "\${WORKSPACE}/llvm-test-output-${targetSuffix}.xml"
echo "Done running check-${targetSuffix} tests"
"""
        junit healthScaleFactor: 2.0, testResults: "llvm-test-output-${targetSuffix}.xml"
    }
}

def doBuild() {
    String llvmBranch = env.BRANCH_NAME
    stage("Checkout sources") {
        timestamps {
            echo("scm=${scm}")
            llvmRepo = doGit('https://github.com/CTSRD-CHERI/llvm-project.git', llvmBranch, 'llvm-project', 'llvm-project')
            echo("LLVM = ${llvmRepo}")
        }
    }
    stage("Build") {
        updateGithubStatus('Compiling...')
        buildScript = '''#!/usr/bin/env bash
set -xe
# Remove old split-repo checkout
if [ -e "${WORKSPACE}/llvm" ]; then
    rm -rf "${WORKSPACE}/llvm"
fi

#remove old artifacts
rm -fv cheri-*-clang-*.tar.xz

if [ -e "${SDKROOT_DIR}" ]; then
   echo "ERROR, old SDK was not deleted!"
   rm -rf "${SDKROOT_DIR}"
fi
# if [ -e "${WORKSPACE}/llvm-project/Build" ]; then
#   echo "ERROR, old build was not deleted!" && exit 1
# fi

# go to llvm-project, checkout the appropriate branch and create the Build directory
git -C "${WORKSPACE}/llvm-project" rev-parse HEAD

cd "${WORKSPACE}/llvm-project" || exit 1
mkdir -p Build

# Remove some files that may have been created in a previous run:
find "${WORKSPACE}/llvm-project" -name cheri-cap-index.ll.mir -delete || true

# run cmake
cd Build || exit 1
CMAKE_ARGS=("-DCMAKE_INSTALL_PREFIX=${SDKROOT_DIR}" "-DLLVM_OPTIMIZED_TABLEGEN=OFF" "-DLLVM_ENABLE_PROJECTS=clang;lld;llvm" "-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=RISCV")
if [ "$label" == "linux" ] ; then
    export CMAKE_CXX_COMPILER=clang++-6.0
    export CMAKE_C_COMPILER=clang-6.0
else
    export CMAKE_CXX_COMPILER=clang++80
    export CMAKE_C_COMPILER=clang80
fi
CMAKE_ARGS+=("-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}" "-DLLVM_ENABLE_LLD=ON")
# Run lit with python3 to avoid multiprocessing errors
CMAKE_ARGS+=("-DPYTHON_EXECUTABLE=$(which python3)")
'''
    if (TEST_RELEASE_BUILD) {
         buildScript += '''CMAKE_ARGS+=("-DCMAKE_BUILD_TYPE=Release" "-DLLVM_ENABLE_ASSERTIONS=OFF" "-DBUILD_SHARED_LIBS=OFF" "-DLLVM_ENABLE_EXPENSIVE_CHECKS=OFF")'''
    } else {
        // Release build with assertions is a bit faster than a debug build and A LOT smaller
        buildScript += '''
CMAKE_ARGS+=("-DCMAKE_BUILD_TYPE=Release" "-DLLVM_ENABLE_ASSERTIONS=ON")
CMAKE_ARGS+=("-DLLVM_USE_SANITIZER=Address;Undefined")'''
    }
        buildScript += '''
# Also don't set the default target or default sysroot when running tests as it breaks quite a few
# max 1 hour total and max 2 minutes per test
CMAKE_ARGS+=("-DLLVM_LIT_ARGS=--xunit-xml-output ${WORKSPACE}/llvm-test-output.xml --max-time 3600 --timeout 240")

rm -f CMakeCache.txt
cmake -G Ninja "${CMAKE_ARGS[@]}" ../llvm

# build
ninja -v ${JFLAG}

# install
ninja install

# Remove all old JUnit XML files:
rm -fv ${WORKSPACE}/llvm-test-*.xml
'''
        sh buildScript
    }
    runTests('all')
    // No need to rerun the full test suite, only run CHERI-specific  tests for 256
    runTests('all-cheri256-only')

    if (TEST_RELEASE_BUILD)
      return;

    stage("Archive artifacts") {
        updateGithubStatus("Archiving artifacts...")
        sh '''#!/usr/bin/env bash
set -xe

du -sh "${SDKROOT_DIR}"

# create links for the various tools
cd $SDKROOT_DIR/bin
TOOLS="clang clang++ clang-cpp llvm-mc llvm-objdump llvm-readobj llvm-size ld.lld"
for TOOL in $TOOLS ; do
    ln -fs $TOOL cheri-unknown-freebsd-$TOOL
    ln -fs $TOOL mips4-unknown-freebsd-$TOOL
    ln -fs $TOOL mips64-unknown-freebsd-$TOOL
done
# cc, c++ and cpp symlinks are expected by e.g. Qt
ln -fs clang mips64-unknown-freebsd-cc
ln -fs clang cheri-unknown-freebsd-cc
ln -fs clang++ mips64-unknown-freebsd-c++
ln -fs clang++ cheri-unknown-freebsd-c++
ln -fs clang-cpp mips64-unknown-freebsd-cpp
ln -fs clang-cpp cheri-unknown-freebsd-cpp

# create symlinks for llvm-{ar,ranlib,nm,objdump,readelf}
LLVM_SYMLINK_KBINUTILS="ar ranlib nm objdump readelf"
for TOOL in $LLVM_SYMLINK_KBINUTILS ; do
    ln -fs llvm-$TOOL $TOOL
    ln -fs llvm-$TOOL cheri-unknown-freebsd-$TOOL
    ln -fs llvm-$TOOL mips4-unknown-freebsd-$TOOL
    ln -fs llvm-$TOOL mips64-unknown-freebsd-$TOOL
done

# clean & bundle up
cd ${WORKSPACE}
ls -laS "${SDKROOT_DIR}/bin"
tar -cJf cheri-${BRANCH_NAME}-clang-include.tar.xz -C ${SDKROOT_DIR} lib/clang
# We can remove all the libraries because we link them statically (but they need to exist)
truncate -s 0 ${SDKROOT_DIR}/lib/lib*
# remove the binaries that are not needed by downstream jobs (saves a lot of archiving and unpacking time)
(cd ${SDKROOT_DIR}/bin && rm -vf clang-check opt llc lli llvm-lto2 llvm-lto llvm-c-test \\
         llvm-dsymutil llvm-dwp llvm-rc llvm-rtdyld clang-func-mapping clang-refactor clang-rename \\
         llvm-extract llvm-xray llvm-split llvm-cov llvm-symbolizer llvm-dwarfdump \\
         llvm-link llvm-stress llvm-cxxdump llvm-cvtres llvm-cat llvm-as llvm-pdbutil \\
         llvm-diff llvm-modextract llvm-dis llvm-pdbdump llvm-profdata llvm-mt llvm-cfi-verify \\
         llvm-opt-report llvm-bcanalyzer llvm-mcmarkup llvm-lib llvm-ranlib \\
         verify-uselistorder sanstats clang-offload-bundler c-index-test \\
         clang-import-test bugpoint sancov obj2yaml yaml2obj)
# Cmake files need tblgen
truncate -s 0 ${SDKROOT_DIR}/bin/llvm-tblgen
ls -laS "${SDKROOT_DIR}/bin"
# remove more useless stuff
rm -rf ${SDKROOT_DIR}/share
rm -rf ${SDKROOT_DIR}/include

# strip the binaries to save space:
(cd ${SDKROOT_DIR}/bin && for i in *; do if test -L $i; then echo "SYMLINK"; else ./llvm-strip $i || echo "Could not strip $i"; fi; done);
ls -laS "${SDKROOT_DIR}/bin"

cd ${SDKROOT_DIR}/..
tar -cJf "cheri-${BRANCH_NAME}-clang-llvm.tar.xz" `basename ${SDKROOT_DIR}`

# clean up to save some disk space
# rm -rf "${WORKSPACE}/llvm-project/Build"
rm -rf "$SDKROOT_DIR"
'''
        archiveArtifacts artifacts: 'cheri-*-clang-*.tar.xz', onlyIfSuccessful: true
    }
}


def nodeLabel = null
if (env.JOB_NAME.toLowerCase().contains("linux")) {
    nodeLabel = "linux"
} else if (env.JOB_NAME.toLowerCase().contains("freebsd")) {
    nodeLabel = "freebsd"
} else {
    error("Invalid job name: ${env.JOB_NAME}")
}

if (env.JOB_NAME.toLowerCase().contains("nodebug")) {
    TEST_RELEASE_BUILD = true
}



node(nodeLabel) {
    try {
        env.label = nodeLabel
        env.SDKROOT_DIR = "${env.WORKSPACE}/sdk"
        env.JFLAG = "-j 20"
        // env.JFLAG = ""
        doBuild()
        // Scan for compiler warnings
        warnings canComputeNew: false, canResolveRelativePaths: true, consoleParsers: [[parserName: 'Clang (LLVM based)']]
        step([$class: 'AnalysisPublisher', canComputeNew: false])
    } finally {
        // Remove the test binaries to save some disk space and to make typos in
        // test scripts fail the build even if a previous commit created that file
        for (path in [env.SDKROOT_DIR, 'llvm-project/Build/test',
                     'llvm-project/Build/tools/clang/test', 'llvm-project/Build/tools/lld/test']) {
            dir(path) {
                deleteDir()
            }
        }
        // set the final build result so we can update the github status
        try {
            currentBuild.result = currentBuild.currentResult
            updateGithubStatus("Build completed.")
        }
        catch(Exception e) {
           echo("Failed to set github status: ${e}")
        }
    }
}
