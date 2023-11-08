#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
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
            
            QString teststr = key;
            QDir dir = QFileInfo(key).absoluteDir(); // Create a QDir from the file path.

            

            dir.cdUp(); // Move up from 'bin64'.
            dir.cdUp(); // Move up from the environment folder.
            if (!dir.exists()) {
                //found a sc.exe but the path is invalid, so let's look for another...
                continue;
            }

            resultPath = dir.path(); // This is the path we want          

            break; // Found the desired path, exit the loop.
        }
    }

    //handle some elementary error cases
    if (resultPath.isEmpty()) {
        QMessageBox::warning(nullptr, "Warning", "There is no string. - Abraham Lincoln, 1995\n"
        "Couldn't find where Star Citizen is installed. :(");
        return;
    }

    resultPath = QDir::cleanPath(resultPath) + QDir::separator(); // ensure we tidy up the actual path format if we've gotten this far

    QDir baseDir(resultPath);
    if (!baseDir.exists()) { //Now let's see if the path even exists instead of relying on some rando string.
        QMessageBox::warning(nullptr, "Warning", "The string was a lie, officer.");
        return;
    }


    SCDIR = baseDir; // If we made it this far, we have a valid path. Set it for use in other functions!
}

void searchAndDeleteUserFolder(bool saveKeybinds)
{
    QStringList filesToPreserve = {
        "USER/Client/0/Profiles/default/actionmaps.xml",
        "USER/Client/0/Profiles/default/attributes.xml"
        // Possibly more files in the future
    };

    QDir backupRootDir = SCDIR.filePath("backup"); // < Where we store the backups when applicable. This could be some other safe location.
    if (!backupRootDir.exists()) {
        backupRootDir.mkpath(".");
    }

    QDirIterator it(SCDIR.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {//Walk through each environment folder inside StarCitzien
        QDir subDir(it.next()); 

        // Create a temporary backup directory for the current environment folder
        QString backupSubFolderPath = backupRootDir.filePath(subDir.dirName());
        QDir backupSubFolderDir(backupSubFolderPath); 
        if (!backupSubFolderDir.exists()) { 
            backupSubFolderDir.mkpath(".");
        }

        //// Save specified files ////
        if (saveKeybinds) {
            for (const QString& relativeFilePath : filesToPreserve) { //iterate through the list of files to preserve
                QString fullFilePath = subDir.filePath(relativeFilePath);//get the full path to the file
                QString backupFilePath = backupSubFolderDir.filePath(QFileInfo(relativeFilePath).fileName());//get the full path to the backup file

                // Check if the file exists and then move it
                if (QFile::exists(fullFilePath)) {
                    QFile::copy(fullFilePath, backupFilePath);
                }
            }
        }

        //// Delete the USER folder! ////
        QDir userFolder(subDir.filePath("USER"));
        if (userFolder.exists()) {
            bool success = userFolder.removeRecursively();
            if (!success) {
                QMessageBox::information(nullptr, "That's odd...", "Could not delete USER folder in path: " + userFolder.path());
            }
        }

        //// Restore the saved files ////
        if (saveKeybinds) {
            for (const QString& relativeFilePath : filesToPreserve) {
                QString backupFilePath = backupSubFolderDir.filePath(QFileInfo(relativeFilePath).fileName());
                QString restoreFilePath = subDir.filePath(relativeFilePath);

                // Recreate the necessary subdirectories
                QDir().mkpath(QFileInfo(restoreFilePath).absolutePath());

                // Restore the file
                if (QFile::exists(backupFilePath)) {
                    QFile::copy(backupFilePath, restoreFilePath);
                    QFile::remove(backupFilePath);
                }
            }
        }
    }

    // delete the backup directory
    bool success = backupRootDir.removeRecursively(); //may want to add additional handling for any issues encountere heretofore.

    QString msg = "All possible USER folders have been deleted.";
    if (saveKeybinds) {
        msg += "\nAll keybinds have been returned to where they belong.";
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

    QPushButton cancelButton("Close");

    QCheckBox saveKeybinds("Save Keybinds and Settings");


    layout.addWidget(&deleteStarCitizenButton);
    layout.addWidget(&deleteUserButton);
    layout.addWidget(&saveKeybinds);
    layout.addWidget(&cancelButton);

    saveKeybinds.setCheckState(Qt::Checked);

    setSCDirectory(); //set SCDIR automatically on startup, so any other functions can use it.

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
