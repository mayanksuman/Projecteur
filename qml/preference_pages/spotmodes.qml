// This file is part of Projecteur - https://github.com/jahnf/projecteur - See LICENSE.md and README.md
import QtQuick 2.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

TabView {
    Tab {
        title: "Red"
        Rectangle { color: "red" }
    }
    Tab {
        title: "Blue"
        Rectangle { color: "blue" }
    }
    Tab {
        title: "Green"
        Rectangle { color: "green" }
    }
}
