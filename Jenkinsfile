def build_antd()
{
  docker.image("xsangle/ci-tools:latest-" + env.arch).withRun("bash").inside {
    sh '''
    printenv
    uname -a
    '''
  }
}

pipeline{
  agent { node{ label'master' }}
  options {
    // Limit build history with buildDiscarder option:
    // daysToKeepStr: history is only kept up to this many days.
    // numToKeepStr: only this many build logs are kept.
    // artifactDaysToKeepStr: artifacts are only kept up to this many days.
    // artifactNumToKeepStr: only this many builds have their artifacts kept.
    buildDiscarder(logRotator(numToKeepStr: "1"))
    // Enable timestamps in build log console
    timestamps()
    // Maximum time to run the whole pipeline before canceling it
    timeout(time: 1, unit: 'HOURS')
    // Use Jenkins ANSI Color Plugin for log console
    ansiColor('xterm')
    // Limit build concurrency to 1 per branch
    disableConcurrentBuilds()
  }
  stages
  {
    stage('Build AMD64') {
      steps {
        script{
          env.arch = "arm64"
        }
        build_antd()
      }
    }
    stage('Build ARM64') {
      steps {
        script{
          env.arch = "arm64"
        }
        build_antd()
      }
    }
    stage('Build ARM') {
      steps {
        script{
          env.arch = "arm"
        }
        build_antd()
      }
    }
    stage("Archive") {
      steps{
        script {
            archiveArtifacts artifacts: 'build/', fingerprint: true, onlyIfSuccessful: true
        }
      }
    }
  }
}
