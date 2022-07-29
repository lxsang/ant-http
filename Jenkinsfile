node {
  def remote = [:]
  remote.name = 'workstation'
  remote.host = 'workstation'
  remote.user = 'dany'
  remote.identityFile = '/var/jenkins_home/.ssh/id_rsa'
  remote.allowAnyHosts = true
  remote.agent = false
  remote.logLevel = 'INFO'
  stage('Build') {
    sshCommand remote: remote, command: '\
    cd $(dirname $(find ~/jenkins/workspace/ant-http@script -name "Jenkinsfile")); \
    libtoolize \
    aclocal \
    autoconf \
    automake --add-missing \
    make \
    '
  }
}
