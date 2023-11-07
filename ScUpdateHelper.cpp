#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <Windows.h>
#include <QDir>
#include <QDirIterator>
char SCPATH[];
QString getSCPathString()
{
    HKEY hKey;
    LPCWSTR subkey = L"SOFTWARE\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache";
    LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_READ, &hKey);

    QString resultPath;

    if (openRes == ERROR_SUCCESS) {
        DWORD index = 0;
        WCHAR valueName[256];
        DWORD valueNameSize = 256;
        WCHAR data[1024];
        DWORD dataSize = 1024;

        while (RegEnumValue(hKey, index, valueName, &valueNameSize, NULL, NULL, (LPBYTE)&data, &dataSize) == ERROR_SUCCESS) {
            QString qValueName = QString::fromWCharArray(valueName);
            if (qValueName.contains("starcitizen.exe", Qt::CaseInsensitive)) {
                int bin64Index = qValueName.indexOf("\\bin64\\", 0, Qt::CaseInsensitive);
                if (bin64Index != -1) {
                    resultPath = qValueName.left(bin64Index); // Get the path before \bin64 (this will include an environment folder!) 
                    resultPath = resultPath.left(resultPath.lastIndexOf("\\")); //so we clip that environment off to get the actual SC folder.
                    break; // Pack it up, we've got a winner.
                }
            }
            valueNameSize = 256;
            dataSize = 1024;
            index++;
        }

        RegCloseKey(hKey);
    }

    return resultPath; // return whatever string we found (could be empty too, handled later)
}
void copyActionMapsFile()
{
	 QDirIterator it(baseDir.path(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        QString subDirPath = it.filePath();

        QString userFolderPath = subDirPath + QDir::separator() + "USER/Client/0/Profiles/default/actionmaps.xml"; 
        QDir userFolder(userFolderPath);
    QString userFolderPath = subDirPath + QDir::separator() + "/actionmaps.xml";

    if (QFile::copy(sourceFilePath, destinationFilePath)) {
        QMessageBox::information(nullptr, "Copy Successful", "Keybinds copied to the root directory.");
    } else {
        QMessageBox::critical(nullptr, "Copy Failed", "Failed to copy Keybinds.");
    }
}



void searchAndDeleteUserFolder()
{
    QString scPath = getSCPathString(); //use the fancy reg query to grab a SC path string

    //handle some elementary error cases
    if (scPath.isEmpty()) {
        QMessageBox::warning(nullptr, "Warning", "There is no string. - Abraham Lincoln, 1995");
        return;
    }

    scPath = QDir::cleanPath(scPath) + QDir::separator(); // ensure we tidy up the actual path format if we've gotten this far

    QDir baseDir(scPath);
    if (!baseDir.exists()) { //Now let's see if the path even exists instead of relying on some rando string.
        QMessageBox::warning(nullptr, "Warning", "The string was a lie, officer.");
        return;
    }

    // So we've verified the directory exists, now let's dig for the USER folders.
    QDirIterator it(baseDir.path(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        QString subDirPath = it.filePath();//Dig through each of the environment folders, hands-free!

        QString userFolderPath = subDirPath + QDir::separator() + "USER"; //slap a hypothetical USER on the path...
        QDir userFolder(userFolderPath);

        if (userFolder.exists()) { //and see what that gets us
            if (!userFolder.removeRecursively()) {
                QMessageBox::information(nullptr, "That's odd...", "Could not delete USER folder in path: " + userFolderPath); //maybe not even needed. USER could alredy be delet, or some weird permissions...
                //we don't return here. There may be more USERS amogus. (sus)
            }
        }
    }

    QMessageBox::information(nullptr, "Operation Complete", "All possible USER folders have been deleted.");
}
	void pasteActionMapsFile()
{
 QDirIterator it(baseDir.path(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        QString subDirPath = it.filePath();

        QString userFolderPath = subDirPath + QDir::separator() + "/actionmaps.xml"; 
        QDir userFolder(userFolderPath);
    QString userFolderPath = subDirPath + QDir::separator() + "LIVE/USER/Client/0/Profiles/default/actionmaps.xml";  

    if (QFile::copy(sourceFilePath, destinationFilePath)) {
        QMessageBox::information(nullptr, "Paste Successful", "Keybinds pasted back to its original location.");
    } else {
        QMessageBox::critical(nullptr, "Paste Failed", "Do you happen to have any Elmers glue.");
    }
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
    layout.addWidget(&deleteStarCitizenButton);
    layout.addWidget(&copyButton);
    layout.addWidget(&deleteUserButton);
    layout.addWidget(&pasteButton);
    layout.addWidget(&cancelButton);


    QObject::connect(&deleteStarCitizenButton, &QPushButton::clicked, [&]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(&window, "Confirmation", "This operation will delete your shaders. Are you sure you want to proceed?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            QDir dir(qgetenv("LOCALAPPDATA") + "/Star Citizen");
            dir.removeRecursively();
        }
        });
	QObject::connect(&copyButton, &QPushButton::clicked, copyActionMapsFile);
	
    QObject::connect(&pasteButton, &QPushButton::clicked, pasteActionMapsFile);

    QObject::connect(&deleteUserButton, &QPushButton::clicked, [&]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(&window, "Confirmation", "This operation will delete USER folders Please back up your keybindings. Are you sure you want to proceed?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            searchAndDeleteUserFolder();
        }
		});

    QObject::connect(&cancelButton, &QPushButton::clicked, [&]() {
        QApplication::quit();
        });

    window.show();
    return app.exec();
}
