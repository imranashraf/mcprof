#include "globals.h"
#include "commatrix.h"

Matrix2D::Matrix2D()
{
    IDNoType cols=DEFAULT_SIZE;
    IDNoType rows=DEFAULT_SIZE;
    float value=0.0f;
    D1ECHO("Default Comm Matrix Size = " << DEFAULT_SIZE);
    Matrix.resize( cols , vector<float>( rows , value) );
}

Matrix2D::Matrix2D(IDNoType size)
{
    IDNoType cols=size;
    IDNoType rows=size;
    float value=0.0f;
    Matrix.resize( cols , vector<float>( rows , value) );
}

void Matrix2D::RecordCommunication(IDNoType prod, IDNoType cons, u32 size)
{
    D2ECHO("Recording Communication b/w " << FUNC(prod) << " and "
        << FUNC(cons) << " of size: " << size );

    if( prod < Matrix.size() && cons < Matrix.size() )
        Matrix[prod][cons] += size;
}

float Matrix2D::MaxCommunication()
{
    float currmax=0.0f;
    // Following iterations can be optimized to TotalSymbols instead of size
    for (IDNoType p=0; p<Matrix.size(); p++)
    {
        for (IDNoType c=0; c<Matrix.size(); c++)
        {
            currmax = std::max(currmax, Matrix[p][c]);
        }
    }
    return currmax;
}

void Matrix2D::Print(ostream &fout)
{
    IDNoType TotalSymbols = symTable.TotalSymbolCount();
    CHECK_LT(TotalSymbols, Matrix.size());

    for (IDNoType p=0; p<TotalSymbols; p++)
    {
        for (IDNoType c=0; c<TotalSymbols; c++)
        {
            fout << setw(12) << Matrix[p][c] <<" ";
        }
        fout<<endl;
    }
}

void Matrix2D::PrintMatrix(ostream &fout)
{
    ECHO("Printing communication matrix as table in text");
    IDNoType TotalSymbols = symTable.TotalSymbolCount();
    CHECK_LT(TotalSymbols, Matrix.size());

    fout << setw(25) << " ";
    for (IDNoType c=0; c<TotalSymbols; c++)
    {
        fout << setw(25) << symTable.GetSymName(c);
    }
    fout << endl;

    for (IDNoType p=0; p<TotalSymbols; p++)
    {
        fout << setw(25) << symTable.GetSymName(p);

        for (IDNoType c=0; c<TotalSymbols; c++)
        {
            fout << setw(25) << Matrix[p][c];
        }
        fout<<endl;
    }
}

void Matrix2D::PrintDot(ostream &dotout)
{
    ECHO("Printing communication in DOT");
    IDNoType TotalSymbols = symTable.TotalSymbolCount();
    CHECK_LT(TotalSymbols, Matrix.size());

    dotout << "digraph {\ngraph [];"
//                << "\nnode [fontcolor=black, style=filled, fontsize=20];"
            << "\nedge [fontsize=14, arrowhead=vee, arrowsize=0.5];"
            << endl;

    string objNodeStyle("fontcolor=black, shape=box, fontsize=20");
    string ftnNodeStyle("fontcolor=black, style=filled, fontsize=20");
    for (IDNoType c=0; c<TotalSymbols; c++)
    {
        if ( symTable.SymIsObj(c) )
            dotout << "\"" << (u16)c << "\"" << " [label=\"" << symTable.GetSymName(c) << "\"" << objNodeStyle << "];" << endl;
        else
            dotout << "\"" << (u16)c << "\"" << " [label=\"" << symTable.GetSymName(c) << "\"" << ftnNodeStyle << "];" << endl;
    }

    int color;
    float maxComm = MaxCommunication();

    for (IDNoType p=0; p<TotalSymbols; p++)
    {
        for (IDNoType c=0; c<TotalSymbols; c++)
        {
            float comm = Matrix[p][c];
            if(comm > 0 )
            {
                color = (int) (  1023 *  log((float)(comm)) / log((float)maxComm)  );
                dotout << dec
                        << "\"" << (u16)p << "\""
                        << "->"
                        << "\"" << (u16)c << "\""
                        << "[label=\""
                        << comm <<" Bytes\""
                        << "color = \"#"
                        << hex
                        << setw(2) << setfill('0') << max(0, color-768)
                        << setw(2) << setfill('0') << min(255, 512-(int)abs(color-512))
                        << setw(2) << setfill('0') << max(0, min(255,512-color))
                        << "\""
                        << "]"
                        << endl;
            }
        }
    }

    dotout << "}" << endl;
}
