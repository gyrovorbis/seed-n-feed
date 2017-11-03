#include <QDebug>
#include <QFile>
#include "model/sql_table.h"

//Stolen from: https://evileg.com/en/post/190/
//thanks, nigga
bool SqlTableModel::exportCSV(const QAbstractItemModel* model, QString filePath) {
    /* Create a CSV file of the object and specify the path to the file
      * Be sure to include a valid path and extension .csv
      * */
     QFile csvFile(filePath);

     // Open or create the file if it does not exist
     if(csvFile.open( QIODevice::WriteOnly ))
     {
         // Create a text stream, which will write the data
         QTextStream textStream( &csvFile );
         QStringList stringList; // The helper object QSqtringList, which will form a line

         for(int row = 0; row < model->rowCount(); row++ )
         {
             stringList.clear();
             for( int column = 0; column < model->columnCount(); column++ )
             {
                 // Write in stringList each table entry
                 stringList << model->data(model->index(row, column)).toString();
             }
             /* Then send the entire file stringList by adding text flow dividers
              * in the form of "" and putting at the end of a line terminator
              * */
             textStream << stringList.join( ',' )+"\n";
         }
         // Close the file - ready
         csvFile.close();
         return true;
     } else {

         qCritical() << "File could not be opened for writing: " << filePath;
         return false;
     }

}


bool SqlTableModel::importCSV(QAbstractItemModel* model, QString filePath) {
    return false;
}

bool SqlTableModel::exportCSV(QString filePath) const {
    return exportCSV(this, filePath);
}

bool SqlTableModel::importCSV(QString filePath) {
    return importCSV(this, filePath);
}
