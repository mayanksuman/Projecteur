import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

ApplicationWindow {
    id: window
    width: 900
    height: 520
    visible: true
    // to be made 'Application Name - Preferences'
    title: qsTr("Preferences")

    //! [orientation]
    readonly property bool inPortrait: window.width < window.height
    //! [orientation]

    ListModel {
        id: option_names
        ListElement {
            name: "General"
            cur_options: ""
            file_to_load: "preference_pages/general.qml"
        }
        ListElement {
            name: "Timer"
            cur_options: "    Clock; Next Timer at ??"
            file_to_load: "preference_pages/timer.qml"
        }
        ListElement {
            name: "SpotLight Modes"
            cur_options: "    Selected Modes: ??\n    Currently active: ??"
            file_to_load: "preference_pages/spotmodes.qml"
        }
        ListElement {
            name: "Next/Back Onhold"
            cur_options: "    Current Next hold: ??\n    Current Back Hold: ??"
            file_to_load: "preference_pages/next_back_hold.qml"
        }
        ListElement {
            name: "About"
            cur_options: ""
            file_to_load: "preference_pages/about.qml"
        }
    }

    Drawer {
        id: option_sidebar
        width: window.width / 3
        height: window.height
        modal: inPortrait
        interactive: inPortrait
        position: inPortrait ? 0 : 1
        visible: !inPortrait

        Component {
            id: highlight_option
            Rectangle {
                width: option_list.width; height: option_list.currentItem.height
                color: "lightsteelblue"; radius: 5
                y: option_list.currentItem.y
                Behavior on y {
                    SpringAnimation {
                        spring: 3
                        damping: 0.2
                    }
                }
            }
        }

        ListView {
            id: option_list
            anchors.fill: parent
            highlight: highlight_option
            highlightFollowsCurrentItem: false
            focus: true

            model: option_names

            delegate: Component {
                        Item {
                            width: parent.width
                            height: 60
                            Column {
                                Text { text: '<b>' + name + ' </b>' }
                                Text { text: cur_options }
                                Text { id:filename;text: qsTr("%1").arg(file_to_load); visible:false }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {option_list.currentIndex = index;
                                    option_content.sourceComponent = Qt.createComponent(filename.text);}
                            }
                        }
                    }

            ScrollIndicator.vertical: ScrollIndicator { }
        }

    }

    Flickable {
        id: preference_page

        anchors.fill: parent
        anchors.leftMargin: !inPortrait ? option_sidebar.width : undefined

        topMargin: 20
        bottomMargin: 20
        contentHeight: option_sidebar.height

        Loader {
            id: option_content
            width: parent.width;  height: parent.height
            visible: true; enabled: false
        }

        ScrollIndicator.vertical: ScrollIndicator { }
    }
}
