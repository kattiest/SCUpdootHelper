#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <Windows.h>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QCheckBox>
#include <QFile>
#include <QLabel>


QDir SCDIR;

void setSCDirectory()
{
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache",
        QSettings::NativeFormat);

    QStringList keys = settings.allKeys();
    QString resultPath;

    for (const QString& key : keys) {
        if (key.contains("starcitizen.exe", Qt::CaseInsensitive)) {
            QDir dir = QFileInfo(key).absoluteDir(); // Create a QDir from the file path.
            dir.cdUp(); // Move up from 'bin64'.
            dir.cdUp(); // Move up from 'LIVE' (or any other folder that 'bin64' was in).

            resultPath = dir.absolutePath(); // This is the path we want.
            break; // Found the desired path, exit the loop.
        }
    }

    //handle some elementary error cases
    if (resultPath.isEmpty()) {
        QMessageBox::warning(nullptr, "Warning", "There is no string. - Abraham Lincoln, 1995");
        return;
    }

    resultPath = QDir::cleanPath(resultPath) + QDir::separator(); // ensure we tidy up the actual path format if we've gotten this far

    QDir baseDir(resultPath);
    if (!baseDir.exists()) { //Now let's see if the path even exists instead of relying on some rando string.
        QMessageBox::warning(nullptr, "Warning", "The string was a lie, officer.");
        return;
    }


    SCDIR = baseDir; // If we made it this far, we have a valid path. Return it.
 
    //QMessageBox::information(nullptr, "Path Found", "The path to Star Citizen is: " + resultPath);

 
}




void searchAndDeleteUserFolder(BOOL saveKeybinds)
{
    QString userSub = "USER";
    QString actionMapsPath = "USER/Client/0/Profiles/default/";
    QString actionMapsFile = "actionmaps.xml";

    //full path to USER folder
    QDir userFolder(SCDIR.filePath(userSub));


    QDirIterator it(SCDIR.path(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        QString subDirPath = it.filePath();

        
        QDir actionMapsDir(subDirPath + QDir::separator() + actionMapsPath);
        QDir actionMapsTempDir(subDirPath + QDir::separator());

        //copy the actionMapsFile from the ActionMapsDir to the current subDirPath (if saveKeybinds is checked and xml dir exists)
        BOOL success = QFile::copy(actionMapsDir.filePath(actionMapsFile), actionMapsTempDir.filePath(actionMapsFile));
        //BOOL success = true; // Debug
        if (actionMapsDir.exists() && saveKeybinds) {
				if (!success) {
					QMessageBox::information(nullptr, "That's odd...", "Could not copy actionmaps.xml from " + actionMapsDir.filePath(actionMapsFile) + "\n to \n" + actionMapsTempDir.filePath(actionMapsFile));
				}			
		}
        
        //then commence deleting the user folder
        QDir userFolder(subDirPath + QDir::separator() + "USER");
        if (userFolder.exists()) {
            BOOL success2 = userFolder.removeRecursively(); // <<<------------ This is the delety part
            //BOOL success2 = true; // Debug with this if no want delet
            if (!success2) {
                QMessageBox::information(nullptr, "That's odd...", "Could not delete USER folder in path: " + userFolder.path()); //maybe not even needed. USER could alredy be delet, or some weird permissions...
            }
        }

        //if saveKeybinds is checked, move the actionmaps.xml file back to the USER folder
        if (saveKeybinds) {

            //since the actionMapsDir is now deleted, we need to recreate it (this also creates the USER folder)
            BOOL success3 = actionMapsTempDir.mkpath(actionMapsPath);
            //BOOL success3=true; // Debug
            if (!success3) {
				QMessageBox::information(nullptr, "Oops", "Could not create directory: " + actionMapsTempDir.path() + QDir::separator() + actionMapsPath);
			}
            //then copy the actionmaps.xml file back to the proper location
            BOOL success4 = QFile::copy(actionMapsTempDir.filePath(actionMapsFile), actionMapsDir.filePath(actionMapsFile));
            //BOOL success4 = true; // Debug
            if (!success4) {
				QMessageBox::information(nullptr, "Oops", "Could not copy actionmaps.xml from " + actionMapsTempDir.filePath(actionMapsFile) + "\n to \n" + actionMapsDir.filePath(actionMapsFile));
            }
            else {
                
                //QMessageBox::information(nullptr, "Debug", "Debug: 'removed' the temporary actionmaps file"); //debug
				QFile::remove(actionMapsTempDir.filePath(actionMapsFile)); //we hit this if we DID copy the file back to it's proper place, so we can delete the temp file now.
			}
		}
    }

    QString msg = "All possible USER folders have been deleted.";
    if (saveKeybinds) {
		msg += "\n and all keybinds have been returned to where they belong.";
	}
    QMessageBox::information(nullptr, "Operation Complete", msg);
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget window;
    QVBoxLayout layout(&window);
    QPushButton deleteStarCitizenButton("Delete Star Citizen Folders");
    QPushButton deleteUserButton("Delete USER Folders");
    QPushButton copyButton("Copy Keybinds to Root Directory");
    QPushButton pasteButton("Paste Keybinds back");
    QPushButton cancelButton("Close");
    //add a text label
    


    QCheckBox saveKeybinds("Save Keybinds");


    layout.addWidget(&deleteStarCitizenButton);
    //layout.addWidget(&copyButton);
    layout.addWidget(&deleteUserButton);
    layout.addWidget(&saveKeybinds);
    //layout.addWidget(&pasteButton);
    layout.addWidget(&cancelButton);
    
    saveKeybinds.setCheckState(Qt::Checked);


    setSCDirectory(); //set this automatically on startup, so any other functions can use it.

    QLabel scDirLabel("SC Folder: " + SCDIR.path());
    layout.addWidget(&scDirLabel);


    QObject::connect(&deleteStarCitizenButton, &QPushButton::clicked, [&]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(&window, "Confirmation", "This operation will delete your shaders. Are you sure you want to proceed?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            QDir dir(qgetenv("LOCALAPPDATA") + "/Star Citizen");
            dir.removeRecursively();
        }
        });

    QObject::connect(&deleteUserButton, &QPushButton::clicked, [&]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(&window, "Confirmation", "This operation will delete USER folders Please back up your keybindings. Are you sure you want to proceed?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            searchAndDeleteUserFolder(saveKeybinds.isChecked());
        }
        });

    QObject::connect(&cancelButton, &QPushButton::clicked, [&]() {
        QApplication::quit();
        });

    window.show();
    return app.exec();
}
