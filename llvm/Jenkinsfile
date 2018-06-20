@Library('ctsrd-jenkins-scripts') _

properties([disableConcurrentBuilds(),
            compressBuildLog(),
            disableResume(),
            [$class: 'GithubProjectProperty', displayName: '', projectUrlStr: 'https://github.com/CTSRD-CHERI/llvm/'],
            [$class: 'CopyArtifactPermissionProperty', projectNames: '*'],
            [$class: 'JobPropertyImpl', throttle: [count: 2, durationName: 'hour', userBoost: true]],
            durabilityHint('PERFORMANCE_OPTIMIZED'),
            pipelineTriggers([githubPush()])
])

// global vars needed to update github status
llvmRepo = null
clangRepo = null
lldRepo = null

def updateGithubStatus(String message) {
    for (repo in [llvmRepo, clangRepo, lldRepo])  {
        setGitHubStatus(repo, [message: message])
    }
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
        updateGithubStatus("Running check-all-${targetSuffix} tests...")
        sh """#!/usr/bin/env bash
set -xe

cd \${WORKSPACE}/llvm-build
# run tests
rm -fv "\${WORKSPACE}/llvm-test-output.xml"
ninja check-all-{targetSuffix} \${JFLAG} || echo "Some ${targetSuffix} tests failed!"
mv -fv "\${WORKSPACE}/llvm-test-output.xml" "\${WORKSPACE}/llvm-test-output-${targetSuffix}.xml"
echo "Done running {targetSuffix} tests"
"""
        junit healthScaleFactor: 2.0, testResults: "llvm-test-output-${targetSuffix}.xml"
    }
}

def doBuild() {
    String llvmBranch = env.BRANCH_NAME
    String clangBranch = llvmBranch
    String lldBranch = llvmBranch == 'cap-table' ? 'master' : llvmBranch
    stage("Checkout sources") {
        timestamps {
            echo("scm=${scm}")
            llvmRepo = doGit('https://github.com/CTSRD-CHERI/llvm', llvmBranch, 'llvm', 'llvm')
            echo("LLVM = ${llvmRepo}")
            clangRepo = doGit('https://github.com/CTSRD-CHERI/clang', clangBranch, 'llvm/tools/clang', 'clang')
            echo("CLANG = ${clangRepo}")
            lldRepo = doGit('https://github.com/CTSRD-CHERI/lld', lldBranch, 'llvm/tools/lld', 'lld')
            echo("LLD = ${lldRepo}")
        }
    }
    stage("Build") {
        updateGithubStatus('Compiling...')
        sh '''#!/usr/bin/env bash 
set -xe

#remove old artifacts
rm -fv cheri-*-clang-*.tar.xz

if [ -e "${SDKROOT_DIR}" ]; then
   echo "ERROR, old SDK was not deleted!" && exit 1
fi
# if [ -e "${WORKSPACE}/llvm-build" ]; then
#   echo "ERROR, old build was not deleted!" && exit 1
# fi

# go to llvm, checkout the appropriate branch and create the Build directory
git -C "${WORKSPACE}/llvm" rev-parse HEAD
git -C "${WORKSPACE}/llvm/tools/clang" rev-parse HEAD
git -C "${WORKSPACE}/llvm/tools/lld" rev-parse HEAD

cd "${WORKSPACE}" || exit 1
mkdir -p llvm-build

# run cmake
cd llvm-build || exit 1
CMAKE_ARGS=("-DCMAKE_INSTALL_PREFIX=${SDKROOT_DIR}" "-DLLVM_OPTIMIZED_TABLEGEN=OFF")
if [ "$label" == "linux" ] ; then
    export CMAKE_CXX_COMPILER=clang++-4.0
    export CMAKE_C_COMPILER=clang-4.0
else
    export CMAKE_CXX_COMPILER=clang++40
    export CMAKE_C_COMPILER=clang40
fi
CMAKE_ARGS+=("-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}" "-DLLVM_ENABLE_LLD=ON")

# Release build with assertions is a bit faster than a debug build and A LOT smaller
CMAKE_ARGS+=("-DCMAKE_BUILD_TYPE=Release" "-DLLVM_ENABLE_ASSERTIONS=ON")
# Also don't set the default target or default sysroot when running tests as it breaks quite a few
# max 1 hour total and max 2 minutes per test
CMAKE_ARGS+=("-DLLVM_LIT_ARGS=--xunit-xml-output ${WORKSPACE}/llvm-test-output.xml --max-time 3600 --timeout 240 ${JFLAG}")

rm -f CMakeCache.txt
cmake -G Ninja "${CMAKE_ARGS[@]}" ../llvm

# build
ninja ${JFLAG}

# install
ninja install
'''
    }
    runTests('cheri128')
    // No need to rerun the full test suite, only run CHERI-specific  tests for 256
    runTests('cheri256-only')

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
cd ${SDKROOT_DIR}/..
tar -cJf "cheri-${BRANCH_NAME}-clang-llvm.tar.xz" `basename ${SDKROOT_DIR}`

# clean up to save some disk space
# rm -rf "${WORKSPACE}/llvm-build"
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

node(nodeLabel) {
    try {
        env.label = nodeLabel
        env.SDKROOT_DIR = "${env.WORKSPACE}/sdk"
        env.JFLAG = "-j20"
        doBuild()
        // Scan for compiler warnings
        warnings canComputeNew: false, canResolveRelativePaths: true, consoleParsers: [[parserName: 'Clang (LLVM based)']]
        step([$class: 'AnalysisPublisher', canComputeNew: false])
    } finally {
        // Remove the test binaries to save some disk space and to make typos in
        // test scripts fail the build even if a previous commit created that file
        for (path in ['llvm/Build', env.SDKROOT_DIR, 'llvm-build/test',
                     'llvm-build/tools/clang/test', 'llvm-build/tools/lld/test']) {
            dir(path) {
                deleteDir()
            }
        }
        // set the final build result so we can update the github status
        currentBuild.result = currentBuild.currentResult
        updateGithubStatus("Build completed.")
    }
}
