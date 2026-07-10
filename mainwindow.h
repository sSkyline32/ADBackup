#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QListWidgetItem>
#include <QFileDialog>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui {
class Android_backup;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    QFile logFile;

    void log(const QString text);

private slots:
    void on_dvcsBt_clicked();

    void on_get_pkgList_clicked();

    void on_Rbt1_toggled(bool checked);

    void on_Rbt2_toggled(bool checked);

    void on_Rbt3_toggled(bool checked);

    void on_pkgList_itemChanged(QListWidgetItem *item);

    void on_StartBt_clicked();

    void on_selAll_checkStateChanged(const Qt::CheckState &arg1);

    void on_shLogs_clicked();

    void on_hdLogs_clicked();

    void on_bcm_clicked();

    void on_inm_clicked();

    bool on_selfldr_clicked();

    void on_ifnddvcs_clicked();

    void on_scanfldr_clicked();

    void on_ihdlogs_clicked();

    void on_ishlogs_clicked();

    void on_istartBt_clicked();

    void on_irbc1_toggled(bool checked);

    void on_irbc2_toggled(bool checked);

    void on_irbc3_toggled(bool checked);

    void on_selfldr_2_clicked();

    void on_ipkgList_itemChanged(QListWidgetItem *item);

    void on_iselall_checkStateChanged(const Qt::CheckState &arg1);

    void on_bresel_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Android_backup *ui;
};
#endif // MAINWINDOW_H
