def remote = [:]
remote.name = 'workstation'
remote.host = 'workstation'
remote.user = 'dany'
remote.identityFile = '/var/jenkins_home/.ssh/id_rsa'
remote.allowAnyHosts = true
remote.agent = false
remote.logLevel = 'INFO'

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
    stage('Build') {
      steps {
        sshCommand remote: remote, command: '''
            set -e
            export WORKSPACE=$(realpath "./jenkins/workspace/ant-http")
            cd $WORKSPACE
            [ -d build ] && rm -rf build
            mkdir -p build/etc/systemd/system/
            mkdir -p build/opt/www
            libtoolize
            aclocal
            autoconf
            automake --add-missing
            ./configure --prefix=/usr
            make
            DESTDIR=$WORKSPACE/build make install
            cp  $WORKSPACE/build/usr/etc/antd-config.ini build/opt/www/config.ini.example
          '''
        script {
            // only useful for any master branch
            //if (env.BRANCH_NAME =~ /^master/) {
            archiveArtifacts artifacts: 'build/', fingerprint: true, onlyIfSuccessful: true
            //}
        }
      }
    }
  }
}
