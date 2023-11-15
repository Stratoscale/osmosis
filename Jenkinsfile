pipeline {
    agent {
        node {
            label 'jenkinsSlave'
        }
    }
    options {
        ansiColor('xterm')
        timestamps()
    }
    stages {
        // Init
        stage('Install dependencies') {
            steps {
                sh("docker build . -t osmosis-build")
            }
        }
        // Build
        stage('Build') {
            steps {
                sh("docker run --name osmosis-build -v \$(pwd):/root osmosis-build make -C/root build")
            }
        }
    }
    post{
        always{
            sh("docker rm --force osmosis-build || true")
        }
    }
}
