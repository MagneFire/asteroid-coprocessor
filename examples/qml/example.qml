
import QtQuick 2.0
import org.asteroid.coprocessor 1.0

Item {
    Coprocessor {
        id: coprocessor

        Component.onCompleted: {
            console.log("coprocessor: " + coprocessor.hasCoprocessor)
        }
    }
    Hands {
        id: hands

        Component.onCompleted: {
            console.log("hands: " + hands.hasHands)
        }
    }
}