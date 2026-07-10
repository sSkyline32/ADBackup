#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <unordered_map>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QDir>

using ld = long double;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Android_backup)
{
    ui->setupUi(this);

    auto create_log_dir = [&]() {
        QString path = QCoreApplication::applicationDirPath() + "/logs";

        QDir().mkpath(path);

        path += "/log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss") + ".txt";
        logFile.setFileName(path);
    }; create_log_dir();

    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Cannot create log file");
    }

    ui->bresel->setDisabled(true);
    ui->selAll->setDisabled(true);
    ui->Ins_widget->hide();
    ui->Rbt1->setChecked(true);
    ui->selAll->setCheckState(Qt::Unchecked);
    ui->progressBar->reset();
    ui->LogsOut->setReadOnly(true);
    ui->dvcsOut->setReadOnly(true);
    ui->label_2->hide();
    ui->irbc1->setChecked(true);
    ui->idvcsList->setReadOnly(true);
    ui->ilogsOut->setReadOnly(true);
    ui->label_4->hide();
    ui->iprogressBar->reset();
    ui->selfldr_2->setDisabled(true);
    ui->iselall->setDisabled(true);
    ui->label_3->hide();
}

#include <QProcess>

void MainWindow::log(const QString text) {
    if (logFile.isOpen()) {
        QTextStream stream(&logFile);

        stream << text << "\n";
        stream.flush();
    }
}

void MainWindow::on_dvcsBt_clicked() {
    QProcess process;

    ui->LogsOut->appendPlainText(">>>./adb devices\n");
    log(">>>./adb devices\n");
    process.start("./adb", {"devices"});
    process.waitForFinished();

    QString out = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    ui->dvcsOut->setPlainText(out);
    ui->LogsOut->appendPlainText(err + "\n" + out);
    log(err + "\n" + out);
}

bool get_pkgList_clicked = false;
int argueNum = 0;
void MainWindow::on_Rbt1_toggled(bool checked) {
    if (checked) {
        ui->label->setText("If you need to change packages for backup list you must update this and select what you want.");
        get_pkgList_clicked = false;
        argueNum = 0;
    }
}
void MainWindow::on_Rbt2_toggled(bool checked) {
    if (checked) {
        ui->label->setText("If you need to change packages for backup list you must update this and select what you want.");
        get_pkgList_clicked = false;
        argueNum = 1;
    }
}
void MainWindow::on_Rbt3_toggled(bool checked) {
    if (checked) {
        ui->label->setText("If you need to change packages for backup list you must update this and select what you want.");
        get_pkgList_clicked = false;
        argueNum = 2;
    }
}

std::unordered_map <std::string, bool> pkg_list;
void get_packages(QString& pr_out) {

    QTextStream str_stream(&pr_out);
    while (!str_stream.atEnd()) {
        std::string package;

        auto read_str = [&]() {
            QString line = str_stream.readLine();
            package = line.toStdString();
        };

        read_str();

        package = package.substr(8, int(package.size()) - 8);
        pkg_list[package] = true;
    }
}

bool bcmode_enabled = true;
bool resel_active = false;
long long total_selected = 0;
void MainWindow::on_get_pkgList_clicked() {
    bcmode_enabled = false;
    resel_active = true;
    ui->bresel->setEnabled(resel_active);
    ui->bcMode->setDisabled(true);

    ui->label->setText("");
    ui->pkgList->clear();

    pkg_list.clear();
    get_pkgList_clicked = true;

    if (argueNum == 0) {
        ui->LogsOut->appendPlainText(">>>./adb shell pm list packages\n");

        log(">>>./adb shell pm list packages\n");
        QProcess process;

        process.start("./adb", {"shell", "pm list packages"});
        process.waitForFinished();

        QString pr_out = process.readAllStandardOutput();
        QString pr_err = process.readAllStandardError();

        get_packages(pr_out);
        ui->LogsOut->appendPlainText(pr_err + "\n" + pr_out);
        log(pr_err + "\n" + pr_out);
    } else if (argueNum == 1) {
        ui->LogsOut->appendPlainText(">>>./adb shell pm list packages -3\n");

        log(">>>./adb shell pm list packages -3\n");
        QProcess process;

        process.start("./adb", {"shell", "pm list packages -3"});
        process.waitForFinished();

        QString pr_out = process.readAllStandardOutput();
        QString pr_err = process.readAllStandardError();

        get_packages(pr_out);
        ui->LogsOut->appendPlainText(pr_err + "\n" + pr_out);

        log(pr_err + "\n" + pr_out);
    } else {
        ui->LogsOut->appendPlainText(">>>./adb shell pm list packages -s\n");
        log(">>>./adb shell pm list packages -s\n");
        QProcess process;

        process.start("./adb", {"shell", "pm list packages -s"});
        process.waitForFinished();

        QString pr_out = process.readAllStandardOutput();
        QString pr_err = process.readAllStandardError();

        get_packages(pr_out);
        ui->LogsOut->appendPlainText(pr_err + "\n" + pr_out);

        log(pr_err + "\n" + pr_out);
    }

    for (auto& [name, state] : pkg_list) {
        auto *item = new QListWidgetItem(QString::fromStdString(name));

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(state ? Qt::Checked : Qt::Unchecked);
        ui->pkgList->addItem(item);
    }
    ui->selAll->setDisabled(false);
    ui->selAll->setChecked(true);

    total_selected = pkg_list.size();
}

void MainWindow::on_pkgList_itemChanged(QListWidgetItem *item) {
    pkg_list[item->text().toStdString()] = (item->checkState() == Qt::Checked);
    total_selected += (pkg_list[item->text().toStdString()] ? 1 : -1);

    if (total_selected != pkg_list.size()) ui->selAll->setCheckState(Qt::Unchecked);
    else ui->selAll->setCheckState(Qt::Checked);

    if (total_selected == 0) {
        ui->label->setText("You don't selected packages!");
    } else if (total_selected > 0) {
        ui->label->setText("");
    }
}

void MainWindow::on_bresel_clicked() {
    bcmode_enabled = true;
    pkg_list.clear();
    total_selected = 0;
    get_pkgList_clicked = false;
    ui->pkgList->clear();
    ui->bcMode->setEnabled(true);
    ui->selAll->setCheckState(Qt::Unchecked);
    ui->selAll->setDisabled(true);

    resel_active = false;
    ui->bresel->setDisabled(true);
}

void MainWindow::on_selAll_checkStateChanged(const Qt::CheckState &arg1) {
    if (ui->selAll->isChecked()) {
        ui->pkgList->blockSignals(true);

        for (int i = 0; i < ui->pkgList->count(); ++i) {
            auto *item = ui->pkgList->item(i);

            item->setCheckState(Qt::Checked);
            pkg_list[item->text().toStdString()] = true;
        }

        ui->pkgList->blockSignals(false);
        total_selected = pkg_list.size();
        if (get_pkgList_clicked) ui->label->setText("");
    } else {
        if (total_selected != pkg_list.size()) return;
        ui->pkgList->blockSignals(true);

        for (int i = 0; i < ui->pkgList->count(); ++i) {
            auto *item = ui->pkgList->item(i);

            item->setCheckState(Qt::Unchecked);
            pkg_list[item->text().toStdString()] = false;
        }

        ui->pkgList->blockSignals(false);
        total_selected = 0;
        if (get_pkgList_clicked) ui->label->setText("You don't selected packages!");
    }
}

void MainWindow::on_StartBt_clicked() {
    if (!get_pkgList_clicked) {
        if (argueNum == 0) {
            ui->LogsOut->appendPlainText(">>>./adb shell pm list packages\n");
            QTextCursor cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log("./adb shell pm list packages\n");
            QProcess process;

            process.start("./adb", {"shell", "pm list packages"});
            process.waitForFinished();

            QString pr_out = process.readAllStandardOutput();
            QString pr_err = process.readAllStandardError();

            get_packages(pr_out);
            ui->LogsOut->appendPlainText(pr_err + "\n" + pr_out);
            cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log(pr_err + "\n" + pr_out);
        } else if (argueNum == 1) {
            ui->LogsOut->appendPlainText(">>>./adb shell pm list packages -3\n");
            QTextCursor cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log("./adb shell pm list packages -3\n");
            QProcess process;

            process.start("./adb", {"shell", "pm list packages -3"});
            process.waitForFinished();

            QString pr_out = process.readAllStandardOutput();
            QString pr_err = process.readAllStandardError();

            get_packages(pr_out);
            ui->LogsOut->appendPlainText(pr_err + "\n" + pr_out);
            cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log(pr_err + "\n" + pr_out);
        } else {
            ui->LogsOut->appendPlainText(">>>./adb shell pm list packages -s\n");
            QTextCursor cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log("./adb shell pm list packages -s\n");
            QProcess process;

            process.start("./adb", {"shell", "pm list packages -s"});
            process.waitForFinished();

            QString pr_out = process.readAllStandardOutput();
            QString pr_err = process.readAllStandardError();

            get_packages(pr_out);
            ui->LogsOut->appendPlainText(pr_err + "\n" + pr_out);
            cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log(pr_err + "\n" + pr_out);
        }
        total_selected = pkg_list.size();
        get_pkgList_clicked = true;
    }

    std::string home_path;

    QString hp = QFileDialog::getExistingDirectory(this, "Select a folder", QDir::homePath());
    home_path = hp.toStdString(); if (home_path.empty()) return;
    home_path += "/backup";

    ui->inm->setDisabled(true);
    ui->bcm->setDisabled(true);
    ui->StartBt->setDisabled(true);
    ui->get_pkgList->setDisabled(true);
    ui->selAll->setDisabled(true);
    ui->bcMode->setDisabled(true);
    ui->dvcsBt->setDisabled(true);
    ui->pkgList->setDisabled(true);
    ui->selAll->setDisabled(true);
    ui->bresel->setDisabled(true);

    if (!std::filesystem::exists(home_path)) {
        std::filesystem::create_directory(home_path);
    }
    home_path.push_back('/');

    if (total_selected == 0) {
        ui->label->setText("You don't selected packages!");
        return;
    }

    long long cnt = 0;
    for (const auto& [name, state] : pkg_list) {
        if (!state) continue;

        std::string query = "pm path "; query += name;
        log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
        log("./adb shell " + QString::fromStdString(query));

        QProcess process;

        process.start("./adb", {"shell", query.c_str()});
        process.waitForFinished();

        QString PKGpaths = process.readAllStandardOutput();
        QTextStream Qst(&PKGpaths);

        std::string pkg_path = home_path; pkg_path += name;

        if (!std::filesystem::exists(pkg_path)) {
            std::filesystem::create_directory(pkg_path);
        }

        query = "";
        while (!Qst.atEnd()) {
            std::string now_path;

            auto readStr = [&](std::string& str) {
                QString qstr = Qst.readLine();
                str = qstr.toStdString();
            };

            readStr(now_path);
            now_path = now_path.substr(8, now_path.size() - 8);

            query += now_path; query.push_back(' ');
            query += pkg_path;

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log("./adb pull " + QString::fromStdString(query));
            ui->LogsOut->appendPlainText(">>>./adb pull ");
            QTextCursor cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            ui->LogsOut->insertPlainText(query.c_str());

            process.start("./adb", {"pull", now_path.c_str(), pkg_path.c_str()});
            while (!process.waitForFinished(20)) {
                QCoreApplication::processEvents();
            }

            QString now_out = process.readAllStandardOutput();
            QString now_err = process.readAllStandardError();

            ui->LogsOut->appendPlainText(now_err + "\n" + now_out);
            cursor = ui->LogsOut->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->LogsOut->setTextCursor(cursor);

            log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
            log(now_err + "\n" + now_out);
            query = "";
        }

        ++cnt;
        int percent = ceil(ld(cnt) / (ld(total_selected) / 100));

        if (cnt != total_selected && percent >= 100) percent = 99;
        else if (percent > 100) percent = 100;

        ui->progressBar->setValue(percent);
    }

    ui->inm->setDisabled(false);
    ui->bcm->setDisabled(false);
    ui->StartBt->setDisabled(false);
    ui->get_pkgList->setDisabled(false);
    ui->selAll->setDisabled(false);
    ui->dvcsBt->setDisabled(false);
    ui->pkgList->setDisabled(false);
    ui->selAll->setDisabled(false);
    ui->bresel->setDisabled(true);
    ui->bcMode->setDisabled(false);
    pkg_list.clear();
    ui->pkgList->clear();
    ui->selAll->setCheckState(Qt::Unchecked);
    ui->selAll->setDisabled(true);
}

void MainWindow::on_shLogs_clicked() {
    ui->LogsOut->show();
    ui->label_2->hide();
}

void MainWindow::on_hdLogs_clicked() {
    ui->LogsOut->hide();
    ui->label_2->show();
}

void MainWindow::on_bcm_clicked() {
    ui->Bc_widget->show();
    ui->Ins_widget->hide();
}

void MainWindow::on_inm_clicked() {
    ui->Ins_widget->show();
    ui->Bc_widget->hide();
}

bool selfldr_clicked = false;
bool scanfld_clicked = false;
std::string path_str;
short install_mode = 0;
void MainWindow::on_irbc1_toggled(bool checked) {
    selfldr_clicked = false;
    scanfld_clicked = false;
    path_str.clear();
    install_mode = 0;
    ui->inotc->show();
    ui->selfldr->setText("Select folder");

    ui->scanfldr->setDisabled(false);
    ui->iselall->setDisabled(false);
    ui->ipkgList->setDisabled(false);
}
void MainWindow::on_irbc2_toggled(bool checked) {
    selfldr_clicked = false;
    scanfld_clicked = false;
    path_str.clear();
    install_mode = 1;
    ui->inotc->hide();
    ui->selfldr->setText("Select file");

    ui->scanfldr->setDisabled(true);
    ui->iselall->setDisabled(true);
    ui->ipkgList->setDisabled(true);
}
void MainWindow::on_irbc3_toggled(bool checked) {
    selfldr_clicked = false;
    scanfld_clicked = false;
    path_str.clear();
    install_mode = 2;
    ui->inotc->show();
    ui->selfldr->setText("Select folder");

    ui->scanfldr->setDisabled(true);
    ui->iselall->setDisabled(true);
    ui->ipkgList->setDisabled(true);
}

std::unordered_map <std::string, std::pair <bool, std::vector <std::string>>> pkgs_paths;
bool MainWindow::on_selfldr_clicked() {
    scanfld_clicked = false;

    if (install_mode == 0) path_str = QString(QFileDialog::getExistingDirectory(this, "Select a folder", QDir::homePath())).toStdString();
    else if (install_mode == 1) path_str = QString(QFileDialog::getOpenFileName(this, "Select a file", QDir::homePath())).toStdString();
    else path_str = QString(QFileDialog::getExistingDirectory(this, "Select a folder", QDir::homePath())).toStdString();

    if (path_str.empty()) {
        return false;
    } selfldr_clicked = true;

    ui->imode->setDisabled(true);
    ui->selfldr->setDisabled(true);
    ui->selfldr_2->setDisabled(false);
    return true;
}
void MainWindow::on_selfldr_2_clicked() {
    ui->imode->setDisabled(false);
    ui->selfldr->setDisabled(false);

    pkgs_paths.clear();
    path_str.clear();
    selfldr_clicked = false;
    scanfld_clicked = false;
    ui->ipkgList->clear();
    ui->iselall->setCheckState(Qt::Unchecked);
    ui->selfldr_2->setDisabled(true);
}

void MainWindow::on_ifnddvcs_clicked() {
    QProcess process;

    log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
    log("./adb devices\n");
    ui->ilogsOut->appendPlainText(">>>./adb devices\n");
    QTextCursor cursor = ui->ilogsOut->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->LogsOut->setTextCursor(cursor);

    process.start("./adb", {"devices"});
    process.waitForFinished();

    QString out = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    ui->idvcsList->setPlainText(out);
    ui->ilogsOut->appendPlainText(err + "\n" + out);
    cursor = ui->ilogsOut->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->LogsOut->setTextCursor(cursor);

    log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
    log(err + "\n" + out);
}

long long total_selected1 = 0;
void MainWindow::on_scanfldr_clicked() {
    if (!selfldr_clicked) {
        if (!MainWindow::on_selfldr_clicked()) return;
    }

    ui->iselall->setCheckState(Qt::Checked);
    scanfld_clicked = true;

    ui->iselall->setDisabled(false);

    if (install_mode == 0) {
        std::filesystem::directory_iterator main_pth(path_str.c_str());

        for (auto& object : main_pth) {
            std::string obj_path = object.path().generic_string();

            if (object.is_regular_file()) {
                pkgs_paths[object.path().filename().string()].second.push_back(obj_path);
                pkgs_paths[object.path().filename().string()].first = true;
            } else if (object.is_directory()) {

                for (auto& object1 : std::filesystem::directory_iterator(obj_path.c_str())) {
                    if (object1.is_directory()) continue;

                    std::string obj1_path = object1.path().generic_string();

                    pkgs_paths[object.path().filename().generic_string()].second.push_back(obj1_path);
                }
                pkgs_paths[object.path().filename().generic_string()].first = true;
            }
        }

        for (auto& [name, obj] : pkgs_paths) {
            ++total_selected1;
            auto *item = new QListWidgetItem(QString::fromStdString(name));

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(obj.first ? Qt::Checked : Qt::Unchecked);
            ui->ipkgList->addItem(item);
        }
        ui->iselall->setCheckState(Qt::Checked);
    } else if (install_mode == 1) {
        pkgs_paths[path_str] = {true, {path_str}};
    } else {
        pkgs_paths[path_str].first = true;
        for (auto& obj : std::filesystem::directory_iterator(path_str.c_str())) {
            pkgs_paths[path_str].second.push_back(obj.path().generic_string());
        }
    }
}

void MainWindow::on_ipkgList_itemChanged(QListWidgetItem *item) {
    pkgs_paths[item->text().toStdString()].first = (item->checkState() == Qt::Checked);
    total_selected1 += (pkgs_paths[item->text().toStdString()].first ? 1 : -1);

    if (total_selected1 != pkgs_paths.size()) ui->iselall->setCheckState(Qt::Unchecked);
    else ui->iselall->setCheckState(Qt::Checked);

    if (total_selected1 == 0) {
        ui->label_3->show();
    } else if (total_selected1 > 0) ui->label_3->hide();
}

void MainWindow::on_iselall_checkStateChanged(const Qt::CheckState &arg1) {
    if (ui->iselall->isChecked()) {
        ui->ipkgList->blockSignals(true);

        total_selected1 = 0;
        for (int i = 0; i < ui->ipkgList->count(); ++i) {
            ++total_selected1;
            auto *item = ui->ipkgList->item(i);

            item->setCheckState(Qt::Checked);
            pkgs_paths[item->text().toStdString()].first = true;
        } ui->label_3->hide();

        ui->ipkgList->blockSignals(false);
    } else {
        if (total_selected1 != pkgs_paths.size()) return;
        ui->ipkgList->blockSignals(true);

        total_selected1 = 0;
        for (int i = 0; i < ui->ipkgList->count(); ++i) {
            auto *item = ui->ipkgList->item(i);

            item->setCheckState(Qt::Unchecked);
            pkgs_paths[item->text().toStdString()].first = false;
        } ui->label_3->show();

        ui->ipkgList->blockSignals(false);
    }
}

void MainWindow::on_ihdlogs_clicked() {
    ui->ilogsOut->hide();
    ui->label_4->show();
}
void MainWindow::on_ishlogs_clicked() {
    ui->ilogsOut->show();
    ui->label_4->hide();
}
void MainWindow::on_istartBt_clicked() {
    if (!selfldr_clicked) {
        if (!MainWindow::on_selfldr_clicked()) return;
        selfldr_clicked = true;
    }

    if (!scanfld_clicked) {
        if (install_mode == 0) {
            std::filesystem::directory_iterator main_pth(path_str.c_str());

            for (auto& object : main_pth) {
                std::string obj_path = object.path().generic_string();

                if (object.is_regular_file()) {
                    pkgs_paths[object.path().filename().string()].second.push_back(obj_path);
                    pkgs_paths[object.path().filename().string()].first = true;
                } else if (object.is_directory()) {
                    for (auto& object1 : std::filesystem::directory_iterator(obj_path.c_str())) {
                        if (object1.is_directory()) continue;

                        std::string obj1_path = object1.path().generic_string();

                        pkgs_paths[obj_path].second.push_back(obj1_path);
                    }
                    pkgs_paths[obj_path].first = true;
                }
            }

            total_selected1 = pkgs_paths.size();
        } else if (install_mode == 1) {
            pkgs_paths[path_str] = {true, {path_str}};
            total_selected1 = 1;
        } else {
            pkgs_paths[path_str].first = true;
            for (auto& obj : std::filesystem::directory_iterator(path_str.c_str())) {
                pkgs_paths[path_str].second.push_back(obj.path().generic_string());
            }
            total_selected1 = 1;
        }

        scanfld_clicked = true;
    }

    ui->inm->setDisabled(true);
    ui->bcm->setDisabled(true);
    ui->istartBt->setDisabled(true);
    ui->ipkgList->setDisabled(true);
    ui->selfldr_2->setDisabled(true);
    ui->iselall->setDisabled(true);
    ui->scanfldr->setDisabled(true);
    ui->ifnddvcs->setDisabled(true);

    ld cnt = 0;
    for (const auto& [name, obj] : pkgs_paths) {
        if (!obj.first) continue;

        ui->ilogsOut->appendPlainText(">>>./adb install-multiple ");
        QTextCursor cursor = ui->ilogsOut->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->LogsOut->setTextCursor(cursor);

        QString to_log = "./adb install-multiple ";

        QStringList args {"install-multiple"};
        if (ui->rii->isChecked()) {
            args.push_back("-r");

            ui->ilogsOut->insertPlainText("-r ");
            to_log += "-r ";
        }



        for (const auto& arg : obj.second) {
            args.push_back(QString::fromStdString(arg));

            to_log += args.back(); to_log += " ";
            ui->ilogsOut->insertPlainText(args.back() + " ");

        } ui->ilogsOut->insertPlainText("\n"); to_log += "\n";
        cursor.movePosition(QTextCursor::End);
        ui->LogsOut->setTextCursor(cursor);

        log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
        log(to_log);

        QProcess process;

        process.start("./adb", args);
        while (!process.waitForFinished(20)) {
            QCoreApplication::processEvents();
        }

        QString out = process.readAllStandardOutput();
        QString err = process.readAllStandardError();

        ui->ilogsOut->appendPlainText(err + out);
        cursor = ui->ilogsOut->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->LogsOut->setTextCursor(cursor);

        log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
        log(err + out);

        ++cnt;
        int percent = ceil(ld(cnt) / (ld(total_selected1) / 100));

        if (cnt != total_selected1 && percent >= 100) percent = 99;
        else if (percent > 100) percent = 100;

        ui->iprogressBar->setValue(percent);
    }

    ui->inm->setDisabled(false);
    ui->bcm->setDisabled(false);
    ui->istartBt->setDisabled(false);
    ui->ipkgList->setDisabled(false);
    ui->selfldr_2->setDisabled(false);
    ui->iselall->setDisabled(false);
    ui->scanfldr->setDisabled(false);
    ui->ifnddvcs->setDisabled(false);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QProcess process;

    log("[" + QDateTime::currentDateTime().toString("HH:mm:ss") + "]");
    log("./adb kill-server\n");

    process.start("./adb", {"kill-server"});
    process.waitForFinished();

    event->accept();
}

MainWindow::~MainWindow() {
    delete ui;
}
