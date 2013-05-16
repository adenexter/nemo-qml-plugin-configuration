TEMPLATE = subdirs

equals(QT_MAJOR_VERSION, 4) {
    target.files = auto-qt4/*
    target.path = /opt/tests/nemo-qml-plugins/configuration/auto
}
equals(QT_MAJOR_VERSION, 5) {
    target.files = auto-qt5/*
    target.path = /opt/tests/nemo-qml-plugins/configuration-qt5/auto
}

INSTALLS += target
