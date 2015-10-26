/*
 * Copyright (C) 2015 Marcus Soll
 * This file is part of qnn-single-run.
 *
 * qnn-single-run is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * qnn-single-run is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with qnn-single-run. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QnnSingleRun_H
#define QnnSingleRun_H

#include <QMainWindow>
#include <QString>
#include <QStringListModel>

namespace Ui {
class QnnSingleRun;
}

class QnnSingleRun : public QMainWindow
{
    Q_OBJECT

public:
    explicit QnnSingleRun(QWidget *parent = 0);
    ~QnnSingleRun();

private slots:
    void on_pushButton_clicked();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionQuit_triggered();
    void on_toolButton_gene_clicked();
    void on_toolButton_neuron_clicked();
    void on_toolButton_gas_clicked();

private:
    void showUnknownSelectionWindow(QString s);
    Ui::QnnSingleRun *ui;

    QStringListModel *_nn_model;
    QStringListModel *_sim_model;
};

#endif // QnnSingleRun_H
