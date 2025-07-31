#include "saleshistorydialog.h"

// Include all necessary Qt widget headers for the implementation
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDebug>

SalesHistoryDialog::SalesHistoryDialog(DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent), m_dbManager(dbManager)
{
    setWindowTitle("Sales History & Invoice Details");
    setMinimumSize(800, 600); // Set a reasonable default size

    // Set up all UI elements and layouts
    setupUI();

    // Load the initial data into the invoices table
    populateInvoicesTable();

    // Connect the signal for when a cell is clicked in the invoices table to our slot
    connect(m_invoicesTable, &QTableWidget::cellClicked, this, &SalesHistoryDialog::onInvoiceSelected);

    // If there are invoices, automatically select the first one to show its details
    if (m_invoicesTable->rowCount() > 0) {
        m_invoicesTable->selectRow(0);
        onInvoiceSelected(0, 0); // Manually trigger the slot for the first row
    }
}

void SalesHistoryDialog::setupUI()
{
    // --- Main Layout ---
    // A horizontal layout to split the dialog into a left and right pane
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // --- Left Pane: Invoices List ---
    QGroupBox *invoicesGroup = new QGroupBox("Invoices");
    QVBoxLayout *invoicesLayout = new QVBoxLayout();

    m_invoicesTable = new QTableWidget(this);
    m_invoicesTable->setColumnCount(3);
    m_invoicesTable->setHorizontalHeaderLabels({"ID", "Date of Sale", "Total Amount"});
    m_invoicesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_invoicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_invoicesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_invoicesTable->verticalHeader()->setVisible(false);
    // Hide the raw ID column, as it's for internal use. We'll store it as data.
    m_invoicesTable->setColumnHidden(0, true);
    m_invoicesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    invoicesLayout->addWidget(m_invoicesTable);
    invoicesGroup->setLayout(invoicesLayout);

    // --- Right Pane: Invoice Details ---
    QGroupBox *detailsGroup = new QGroupBox("Invoice Details");
    QVBoxLayout *detailsLayout = new QVBoxLayout();

    m_detailsTable = new QTableWidget(this);
    m_detailsTable->setColumnCount(3);
    m_detailsTable->setHorizontalHeaderLabels({"Medicine Name", "Quantity Sold", "Price at Sale"});
    m_detailsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_detailsTable->verticalHeader()->setVisible(false);
    m_detailsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    detailsLayout->addWidget(m_detailsTable);
    detailsGroup->setLayout(detailsLayout);

    // --- Assemble Panes into Main Layout ---
    // Give more space to the details pane (67%) than the invoice list (33%)
    mainLayout->addWidget(invoicesGroup, 1);
    mainLayout->addWidget(detailsGroup, 2);

    this->setLayout(mainLayout);
}

void SalesHistoryDialog::populateInvoicesTable()
{
    // Ensure the database manager pointer is valid
    if (!m_dbManager) {
        qWarning() << "DatabaseManager not available in SalesHistoryDialog.";
        return;
    }

    // Clear any previous content
    m_invoicesTable->setRowCount(0);

    // Get the list of all invoices from the database
    QList<QVariantList> invoices = m_dbManager->getInvoices();
    m_invoicesTable->setRowCount(invoices.count());

    for (int i = 0; i < invoices.count(); ++i) {
        const QVariantList& invoiceData = invoices.at(i);

        // Column 0: ID (hidden, but we store the ID as data for later use)
        QTableWidgetItem *idItem = new QTableWidgetItem(invoiceData[0].toString());
        idItem->setData(Qt::UserRole, invoiceData[0]); // Store the raw ID in the UserRole
        m_invoicesTable->setItem(i, 0, idItem);

        // Column 1: Date
        QTableWidgetItem *dateItem = new QTableWidgetItem(invoiceData[1].toString());
        m_invoicesTable->setItem(i, 1, dateItem);

        // Column 2: Total Amount (formatted as currency)
        double total = invoiceData[2].toDouble();
        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(total, 'f', 2));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_invoicesTable->setItem(i, 2, totalItem);
    }
}

void SalesHistoryDialog::onInvoiceSelected(int row, int column)
{
    Q_UNUSED(column); // We don't care which column was clicked, only the row

    // Get the item from the first column of the selected row
    QTableWidgetItem *idItem = m_invoicesTable->item(row, 0);
    if (!idItem) return; // Safety check

    // Retrieve the invoice ID we stored in the UserRole
    qint64 invoiceId = idItem->data(Qt::UserRole).toLongLong();

    // Clear the details table before populating it
    m_detailsTable->setRowCount(0);

    // Get the details for the selected invoice from the database
    QList<QVariantList> details = m_dbManager->getInvoiceDetails(invoiceId);
    m_detailsTable->setRowCount(details.count());

    for (int i = 0; i < details.count(); ++i) {
        const QVariantList& detailData = details.at(i);

        // Column 0: Medicine Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(detailData[0].toString());
        m_detailsTable->setItem(i, 0, nameItem);

        // Column 1: Quantity Sold
        QTableWidgetItem *qtyItem = new QTableWidgetItem(detailData[1].toString());
        qtyItem->setTextAlignment(Qt::AlignCenter);
        m_detailsTable->setItem(i, 1, qtyItem);

        // Column 2: Price at Sale (formatted as currency)
        double price = detailData[2].toDouble();
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(price, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_detailsTable->setItem(i, 2, priceItem);
    }
}
