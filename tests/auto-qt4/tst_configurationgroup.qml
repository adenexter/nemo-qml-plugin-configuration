import QtQuickTest 1.0
import QtQuick 1.1
import org.nemomobile.configuration 1.0

Item {
    width: 500; height: 500

    ConfigurationGroup {
        id: settings1

        path: "/opt/tests/org/nemomobile/gconf"

        ConfigurationGroup {
            id: child1

            path: "child1"

            property int integerProperty: 4
            property real realProperty: 12.5
            property bool booleanProperty: false
            property string stringProperty: "default"
            property variant stringListProperty: [ "one", "two", "three" ]
            property variant integerListProperty: [ 1, 2, 3 ]
            property variant pointProperty: Qt.point(12, 34)
            property variant sizeProperty: Qt.size(43, 12)
        }
    }

    ConfigurationGroup {
        id: settings2

        path: "/opt/tests/org/nemomobile/gconf"

        ConfigurationGroup {
            id: child2

            path: "child2"

            property int integerProperty: 4
            property real realProperty: 12.5
            property bool booleanProperty: false
            property string stringProperty: "default"
            property variant stringListProperty: [ "one", "two", "three" ]
            property variant integerListProperty: [ 1, 2, 3 ]
            property variant pointProperty: Qt.point(12, 34)
            property variant sizeProperty: Qt.size(43, 12)
        }

    }

    resources: TestCase {
        name: "ConfigurationGroup"

        function initTestCase() {
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/integerProperty", child2.integerProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/realProperty", child2.realProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/booleanProperty", child2.booleanProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/stringProperty", child2.stringProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/child2/stringListProperty", child2.stringListProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/integerListProperty", child2.integerListProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/pointProperty", child2.pointProperty)
            Configuration.write("/opt/tests/org/nemomobile/gconf/child2/sizeProperty", child2.sizeProperty)
        }

        function cleanup() {
            child1.path = "child1"
            child2.path = "child2"
        }

        function cleanupTestCase() {
            Configuration.clear("/opt/tests/org/nemomobile/gconf")
        }

        function test_integerProperty() {
            child1.integerProperty = 65
            compare(child1.integerProperty, 65)
            compare(child2.integerProperty, 4)

            child2.path = "child1"
            compare(child2.integerProperty, 65)

            child1.integerProperty = 4
            tryCompare(child2, "integerProperty", 4)
        }

        function test_realProperty() {
            child1.realProperty = 0.5
            compare(child1.realProperty, 0.5)
            compare(child2.realProperty, 12.5)

            child2.path = "child1"
            compare(child2.realProperty, 0.5)

            child1.realProperty = 12.5
            tryCompare(child2, "realProperty", 12.5)
        }

        function test_booleanProperty() {
            child1.booleanProperty = true
            compare(child1.booleanProperty, true)
            compare(child2.booleanProperty, false)

            child2.path = "child1"
            compare(child2.booleanProperty, true)

            child1.booleanProperty = false
            tryCompare(child2, "booleanProperty", false)
        }

        function test_stringProperty() {
            child1.stringProperty = "changed"
            compare(child1.stringProperty, "changed")
            compare(child2.stringProperty, "default")

            child2.path = "child1"
            compare(child2.stringProperty, "changed")

            child1.stringProperty = "default"
            tryCompare(child2, "stringProperty", "default")
        }

        function test_stringListProperty() {
            child1.stringListProperty = [ "a", "B", "c", "d" ]
            compare(child1.stringListProperty, [ "a", "B", "c", "d" ])
            compare(child2.stringListProperty, [ "one", "two", "three" ])

            child2.path = "child1"
            compare(child2.stringListProperty, [ "a", "B", "c", "d" ])

            child1.stringListProperty = [ "one", "two", "three" ]
            tryCompare(child2, "stringListProperty", [ "one", "two", "three" ])
        }

        function test_integerListProperty() {
            child1.integerListProperty = [ 8, 7, 5 ]
            compare(child1.integerListProperty, [ 8, 7, 5 ])
            compare(child2.integerListProperty, [ 1, 2, 3 ])

            child2.path = "child1"
            compare(child2.integerListProperty, [ 8, 7, 5 ])

            child1.integerListProperty = [ 1, 2, 3 ]
            tryCompare(child2, "integerListProperty", [ 1, 2, 3 ])
        }

        function test_pointProperty() {
            child1.pointProperty = Qt.point(1, 2)
            compare(child1.pointProperty, Qt.point(1, 2))
            compare(child2.pointProperty, Qt.point(12, 34))

            child2.path = "child1"
            compare(child2.pointProperty, Qt.point(1, 2))

            child1.pointProperty = Qt.point(12, 34)
            tryCompare(child2, "pointProperty", Qt.point(12, 34))
        }

        function test_sizeProperty() {
            child1.sizeProperty = Qt.size(2, 1)
            compare(child1.sizeProperty, Qt.size(2, 1))
            compare(child2.sizeProperty, Qt.size(43, 12))

            child2.path = "child1"
            compare(child2.sizeProperty, Qt.size(2, 1))

            child1.sizeProperty = Qt.size(43, 12)
            tryCompare(child2, "sizeProperty", Qt.size(43, 12))
        }
    }
}
