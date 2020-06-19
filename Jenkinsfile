@Library('ctsrd-jenkins-scripts') _

properties([
        // compressBuildLog(), // Broken, see https://issues.jenkins-ci.org/browse/JENKINS-54680
        disableConcurrentBuilds(),
        disableResume(),
        [$class: 'GithubProjectProperty', displayName: '', projectUrlStr: 'https://github.com/CTSRD-CHERI/llvm-project/'],
        [$class: 'CopyArtifactPermissionProperty', projectNames: '*'],
        [$class: 'JobPropertyImpl', throttle: [count: 2, durationName: 'hour', userBoost: true]],
        durabilityHint('PERFORMANCE_OPTIMIZED'),
        pipelineTriggers([githubPush(), pollSCM('@daily')])
])


def nodeLabel = null
echo("JOB_NAME='${env.JOB_NAME}', JOB_BASE_NAME='${env.JOB_BASE_NAME}'")
if (env.JOB_NAME.toLowerCase().contains("linux")) {
    // Ensure that we can run the resulting binaries on all Linux slaves:
    nodeLabel = "linux-baseline"
} else if (env.JOB_NAME.toLowerCase().contains("freebsd")) {
    nodeLabel = "freebsd"
} else {
    error("Invalid job name: ${env.JOB_NAME}")
}

def TEST_RELEASE_BUILD = false
if (env.JOB_NAME.toLowerCase().contains("nodebug")) {
    TEST_RELEASE_BUILD = true
}
def TEST_WITH_SANITIZERS = false
if (env.JOB_NAME.toLowerCase().contains("sanitizer")) {
    TEST_WITH_SANITIZERS = true
}

cheribuildArgs = [
        '--llvm/build-type=Release', // DEBUG builds are too slow, we use release + assertions
        '--install-prefix=/sdk', // This path is expected by downstream jobs
        '--without-sdk', // Use host compilers
        '--llvm/build-everything', // build all targets
        '--llvm/install-toolchain-only', // but only install compiler+binutils
]
cmakeArgs = [
        // Shared library builds are significantly slower
        '-DBUILD_SHARED_LIBS=OFF',
        // Avoid expensive checks (but keep assertions by default)
        '-DLLVM_ENABLE_EXPENSIVE_CHECKS=OFF',
        // link the C++ standard library statically (if possible)
        // This should make it easier to run binaries on other systems
        '-DLLVM_STATIC_LINK_CXX_STDLIB=ON',
]
if (TEST_RELEASE_BUILD) {
    cheribuildArgs.add('--llvm/no-assertions')
} else {
    // Release build with assertions is a bit faster than a debug build and A LOT smaller
    cheribuildArgs.add('--llvm/assertions')
}
def individualTestTimeout = 300
if (TEST_WITH_SANITIZERS) {
    individualTestTimeout = 600  // Sanitizer builds are slow
    cheribuildArgs.add('--llvm/use-asan')
}
// By default max 1 hour total and max 2 minutes per test
cmakeArgs.add("-DLLVM_LIT_ARGS=--xunit-xml-output \${WORKSPACE}/llvm-test-output.xml --max-time 3600 --timeout ${individualTestTimeout}")
// Quote and join the cmake args
cheribuildCmakeOption = '\'--llvm/cmake-options=\"' + cmakeArgs.join('\" \"') + '\"\''
echo("CMake options = ${cheribuildCmakeOption}")
cheribuildArgs.add(cheribuildCmakeOption)


// Only archive artifacts for the default master and dev builds
def archiveArtifacts = false
if (env.JOB_NAME.startsWith('CLANG-LLVM-linux/') || env.JOB_NAME.startsWith('CLANG-LLVM-freebsd/')) {
    // Skip pull requests and non-default branches:
    def archiveBranches = ['master', 'dev']
    if (!env.CHANGE_ID && (archiveBranches.contains(env.BRANCH_NAME))) {
        archiveArtifacts = true
    }
}

cheribuildProject(target: 'llvm-native', architecture: 'native',
        customGitCheckoutDir: 'llvm-project',
        nodeLabel: nodeLabel,
        fetchCheriCompiler: false, // We are building the CHERI compiler, don't fetch it...
        extraArgs: cheribuildArgs.join(" "),
        skipArchiving: !archiveArtifacts, skipTarball: !archiveArtifacts,
        tarballName: "cheri-clang-llvm.tar.xz",
        runTests: true,
        junitXmlFiles: "llvm-test-output.xml",
)
