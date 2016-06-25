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

#include "qnnsinglerun.h"
#include "ui_qnnsinglerun.h"
#include "additionalsimulationfunctions.hpp"

#include <math.h>
#include <QStringList>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTime>

// NN
#include <network/abstractneuralnetwork.h>
#include <network/feedforwardnetwork.h>
#include <network/continuoustimerecurrenneuralnetwork.h>
#include <network/gasnet.h>
#include <network/modulatedspikingneuronsnetwork.h>

// Genes
#include <network/genericgene.h>
#include <network/lengthchanginggene.h>

// SIM
#include <simulation/abstractsimulation.h>
#include <simulation/tmazesimulation.h>
#include <simulation/rebergrammarsimulation.h>


QnnSingleRun::QnnSingleRun(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QnnSingleRun),
    _nn_model(NULL),
    _sim_model(NULL)
{
    ui->setupUi(this);

    _nn_model = new QStringListModel(this);
    ui->nnView->setModel(_nn_model);
    _sim_model = new QStringListModel(this);
    ui->simView->setModel(_sim_model);

    QStringList nn;
    nn << "FeedForwardNeuralNetwork";
    nn << "FeedForwardNeuralNetwork (tanh)";
    nn << "ContinuousTimeRecurrenNeuralNetwork";
    nn << "ContinuousTimeRecurrenNeuralNetwork (tanh)";
    nn << "ContinuousTimeRecurrenNeuralNetwork (size changing)";
    nn << "ContinuousTimeRecurrenNeuralNetwork (size changing, tanh)";
    nn << "GasNet";
    nn << "ModulatedSpikingNeuronsNetwork (a)";
    nn << "ModulatedSpikingNeuronsNetwork (b)";
    nn << "ModulatedSpikingNeuronsNetwork (c)";
    nn << "ModulatedSpikingNeuronsNetwork (d)";
    nn << "ModulatedSpikingNeuronsNetwork (full)";
    nn << "ModulatedSpikingNeuronsNetwork (none)";

    _nn_model->setStringList(nn);

    QStringList sim;
    sim << "TMazeSimulation";
    sim << "TMazeSimulation (huge)";
    sim << "ReberGrammarSimulation (DetectGrammar)";
    sim << "ReberGrammarSimulation (CreateWords)";
    sim << "ReberGrammarSimulation (embedded, DetectGrammar)";
    sim << "ReberGrammarSimulation (embedded, CreateWords)";

    _sim_model->setStringList(sim);
}

QnnSingleRun::~QnnSingleRun()
{
    delete ui;
}

void QnnSingleRun::on_pushButton_clicked()
{
    AbstractNeuralNetwork *network = NULL;
    AbstractSimulation *simulation = NULL;
    GenericGene *gene;

    bool file_neuron = ui->lineEdit_neuron->text() != "";
    QFile output_neuron(ui->lineEdit_neuron->text());

    bool file_gas = ui->lineEdit_gas->text() != "";
    QFile output_gas(ui->lineEdit_gas->text());

    QMessageBox window;

    QString selection;

    if(ui->simView->currentIndex().data().toString() == ""
            || ui->nnView->currentIndex().data().toString() == "")
    {
        QMessageBox::information(this,
                                 tr("Invalid selection"),
                                 tr("Please select an item from every category"));
        return;
    }

    if(ui->lineEdit_gene->text() == "")
    {
        QMessageBox::information(this,
                                 tr("Invalid selection"),
                                 tr("Please select a gene file"));
        return;
    }

    QFile gene_file(ui->lineEdit_gene->text());

    // Load gene
    if(GenericGene::canLoadThisGene(&gene_file))
    {
        gene = GenericGene::loadThisGene(&gene_file);
    }
    else if(LengthChangingGene::canLoadThisGene(&gene_file))
    {
        gene = LengthChangingGene::loadThisGene(&gene_file);
    }
    else
    {
        QMessageBox::information(this,
                                 tr("Broken gene file"),
                                 tr("Can not load any gene from file."));
        return;
    }

    // parse SIM
    selection = ui->simView->currentIndex().data().toString();
    if(selection == "TMazeSimulation")
    {
        TMazeSimulation::config config;
        config.trials = 1;
        simulation = new TMazeSimulation(config);
    }
    else if(selection == "TMazeSimulation (huge)")
    {
        TMazeSimulation::config config;
        config.max_timesteps = 500;
        config.generateTMaze = &AdditionalSimulationFunctions::generateHugeMaze;
        config.trials = 1;
        simulation = new TMazeSimulation(config);
    }
    else if(selection == "ReberGrammarSimulation (DetectGrammar)")
    {
        ReberGrammarSimulation::config config;
        config.trials_create = 1;
        config.trials_detect = 1;
        simulation = new ReberGrammarSimulation(config);
    }
    else if(selection == "ReberGrammarSimulation (CreateWords)")
    {
        ReberGrammarSimulation::config config;
        config.trials_create = 1;
        config.trials_detect = 1;
        config.mode = ReberGrammarSimulation::CreateWords;
        simulation = new ReberGrammarSimulation(config);
    }
    else if(selection == "ReberGrammarSimulation (embedded, DetectGrammar)")
    {
        ReberGrammarSimulation::config config;
        config.trials_create = 1;
        config.trials_detect = 1;
        config.embedded = true;
        simulation = new ReberGrammarSimulation(config);
    }
    else if(selection == "ReberGrammarSimulation (embedded, CreateWords)")
    {
        ReberGrammarSimulation::config config;
        config.trials_create = 1;
        config.trials_detect = 1;
        config.mode = ReberGrammarSimulation::CreateWords;
        config.embedded = true;
        simulation = new ReberGrammarSimulation(config);
    }
    else
    {
        showUnknownSelectionWindow(selection);
        goto on_pushButton_clicked_cleanup;
    }

    // parse NN
    selection = ui->nnView->currentIndex().data().toString();
    if(selection == "FeedForwardNeuralNetwork")
    {
        network = new FeedForwardNetwork(simulation->needInputLength(), simulation->needOutputLength());
    }
    else if(selection == "FeedForwardNeuralNetwork (tanh)")
    {
        FeedForwardNetwork::config config;
        config.activision_function = &tanh;
        network = new FeedForwardNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ContinuousTimeRecurrenNeuralNetwork")
    {
        ContinuousTimeRecurrenNeuralNetwork::config config;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        network = new ContinuousTimeRecurrenNeuralNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ContinuousTimeRecurrenNeuralNetwork (tanh)")
    {
        ContinuousTimeRecurrenNeuralNetwork::config config;
        config.activision_function = &tanh;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        network = new ContinuousTimeRecurrenNeuralNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ContinuousTimeRecurrenNeuralNetwork (size changing)")
    {
        ContinuousTimeRecurrenNeuralNetwork::config config;
        config.size_changing = true;
        config.network_default_size_grow = 1;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        network = new ContinuousTimeRecurrenNeuralNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ContinuousTimeRecurrenNeuralNetwork (size changing, tanh)")
    {
        ContinuousTimeRecurrenNeuralNetwork::config config;
        config.size_changing = true;
        config.network_default_size_grow = 1;
        config.activision_function = &tanh;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        network = new ContinuousTimeRecurrenNeuralNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "GasNet")
    {
        GasNet::config config;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new GasNet(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ModulatedSpikingNeuronsNetwork (a)")
    {
        ModulatedSpikingNeuronsNetwork::config config;
        config.a_modulated = true;
        config.b_modulated = false;
        config.c_modulated = false;
        config.d_modulated = false;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new ModulatedSpikingNeuronsNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ModulatedSpikingNeuronsNetwork (b)")
    {
        ModulatedSpikingNeuronsNetwork::config config;
        config.a_modulated = false;
        config.b_modulated = true;
        config.c_modulated = false;
        config.d_modulated = false;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new ModulatedSpikingNeuronsNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ModulatedSpikingNeuronsNetwork (c)")
    {
        ModulatedSpikingNeuronsNetwork::config config;
        config.a_modulated = false;
        config.b_modulated = false;
        config.c_modulated = true;
        config.d_modulated = false;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new ModulatedSpikingNeuronsNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ModulatedSpikingNeuronsNetwork (d)")
    {
        ModulatedSpikingNeuronsNetwork::config config;
        config.a_modulated = false;
        config.b_modulated = false;
        config.c_modulated = false;
        config.d_modulated = true;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new ModulatedSpikingNeuronsNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ModulatedSpikingNeuronsNetwork (full)")
    {
        ModulatedSpikingNeuronsNetwork::config config;
        config.a_modulated = true;
        config.b_modulated = true;
        config.c_modulated = true;
        config.d_modulated = true;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new ModulatedSpikingNeuronsNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else if(selection == "ModulatedSpikingNeuronsNetwork (none)")
    {
        ModulatedSpikingNeuronsNetwork::config config;
        config.a_modulated = false;
        config.b_modulated = false;
        config.c_modulated = false;
        config.d_modulated = false;
        if(file_neuron)
        {
            config.neuron_save = &output_neuron;
        }
        if(file_gas)
        {
            config.gas_save = &output_gas;
        }
        network = new ModulatedSpikingNeuronsNetwork(simulation->needInputLength(), simulation->needOutputLength(), config);
    }
    else
    {
        showUnknownSelectionWindow(selection);
        goto on_pushButton_clicked_cleanup;
    }

    simulation->initialise(network, gene);

    window.setWindowTitle(tr("Running simulation"));
    window.setText(tr("The current simulation is running"));
    window.setWindowFlags(((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint));
    window.show();
    simulation->getScore();
    window.close();

    // cleanup
on_pushButton_clicked_cleanup:

    delete simulation;
    delete network;
    delete gene;
}

void QnnSingleRun::on_actionAbout_triggered()
{
    QMessageBox::information(this,
                             tr("About QnnSingleRun"),
                             tr("QnnSingleRun is a simple graphical interface to the qnn library\nAuthor: Marcus Soll\nLicense: GPL3+\nThis program uses qnn, which is licensed under the LGPL3+"));
}

void QnnSingleRun::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void QnnSingleRun::on_actionQuit_triggered()
{
    QApplication::quit();
}

void QnnSingleRun::showUnknownSelectionWindow(QString s)
{
    QMessageBox::warning(this,
                         tr("Unknown selection"),
                         QString(tr("Unknown selection: &1")).arg(s));
}

void QnnSingleRun::on_toolButton_gene_clicked()
{
    QFileDialog dialog(this, tr("Open gene file"), "", "gene file (*.gene)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if(dialog.exec() == QFileDialog::Accepted && dialog.selectedFiles()[0].length() > 0)
    {
        ui->lineEdit_gene->setText(dialog.selectedFiles()[0]);
    }
}

void QnnSingleRun::on_toolButton_neuron_clicked()
{
    QFileDialog dialog(this, tr("Open output file"), "", "CSV file (*.csv)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDefaultSuffix("csv");
    if(dialog.exec() == QFileDialog::Accepted && dialog.selectedFiles()[0].length() > 0)
    {
        ui->lineEdit_neuron->setText(dialog.selectedFiles()[0]);
    }
}

void QnnSingleRun::on_toolButton_gas_clicked()
{
    QFileDialog dialog(this, tr("Open output file"), "", "CSV file (*.csv)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDefaultSuffix("csv");
    if(dialog.exec() == QFileDialog::Accepted && dialog.selectedFiles()[0].length() > 0)
    {
        ui->lineEdit_gas->setText(dialog.selectedFiles()[0]);
    }
}
