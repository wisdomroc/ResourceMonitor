#ifndef COMBOBOX_H
#define COMBOBOX_H


#include <QComboBox>
#include <QListView>

class ComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit ComboBox(QWidget *parent = nullptr);
    bool eventFilter(QObject * watched, QEvent * event) override;
    void hidePopup() override;
    void showPopup() override;
    void setDelimiter(QString str){ _delimiter = str; }

private:
    QString _delimiter = ",";

signals:
    void beforeOpen();

public:
    QCompleter *completer_;
};

#endif // COMBOBOX_H
