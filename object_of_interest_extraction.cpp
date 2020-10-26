#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<istream>
#include<algorithm>
#include<iomanip>
using namespace std;

struct ObjIntPos
{
	long row;
	long col;
};

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long long ullong;
char sourceFile[] = "matrix.txt\0";
char destinationFile[] = "outputFile.txt\0";
uint hght, wdth;
uint compression_multiplier = 10;
uint clusterSize = 7; //Should be odd number
bool OnMethodCompleteMsgs = true;

//Converts array of chars
short _8BitNumOfCharArr(const uchar num[], short len)
{
	uchar _8bitNum = 0;
	for (int i = 0; i < len - 1; i++)
	{
		_8bitNum += num[i];
		_8bitNum *= 10;
	}
	return _8bitNum + num[len - 1];
}
void LoadMonoLine(ifstream &source, vector<uchar> &target)
{
	string line;
	getline(source, line);
	uchar num[3];
	short nlen = 0;
	uint i = 0;
	while (i < line.size())
	{
		if (line[i] != ' ' && line[i] != '\n')
		{
			num[nlen++] = line[i] - '0';
		}
		else
		{
			target.push_back(_8BitNumOfCharArr(num, nlen));
			nlen = 0;
		}
		i++;
	}
}
//Load the matrix from source file to target array and sets
void LoadMonoMat(char *source, vector<uchar> &target, uint &hght, uint &wdth)
{
	ifstream mcMat; // creating object to read the monochrome matrix from txt file
	mcMat.open(source);
	LoadMonoLine(mcMat, target);
	wdth = target.size();
	long hight = 1;
	while (!mcMat.eof())
	{
		LoadMonoLine(mcMat, target);
		hight++;
	}
	hght = hight;
	mcMat.close();
	if (OnMethodCompleteMsgs) cout << "Matrix loaded to 2d array" << endl;
}
uchar GetThresholdVal(const vector<uchar> monoMat, int rowInd, int colInd, uint clusterSize, uint height, uint width)
{
	uint tval = 0, elcnt = 0; // Threshold value and element counter
	rowInd = max(0, rowInd);
	colInd = max(0, colInd);
	uint rmrg = min(width, colInd + clusterSize);
	uint bmrg = min(height, rowInd + clusterSize);
	for (uint i = rowInd; i < bmrg; i++)
	{
		for (uint j = colInd; j < rmrg; j++)
		{
			tval += monoMat[i*width + j];
			elcnt++;
		}
	}
	return tval / elcnt;
}
void CompressMat(const vector<uchar> monoMat, vector<uchar> &compMonoMat, uint clusterSize, uint height, uint width)
{
	uint k = 0;
	if (height % clusterSize == 0 && width % clusterSize == 0)
	{
		for (uint i = 0; i < height; i += clusterSize)
		{
			for (uint j = 0; j < width; j += clusterSize)
			{
				compMonoMat[k++] = GetThresholdVal(monoMat, i, j, clusterSize, height, width);
			}
		}
	}
	else
	{
		long bLine = height - (height%clusterSize); //bottom line, as number, not index
		long rLine = width - (width%clusterSize); //right line, as number, not index
		for (uint i = 0; i < bLine; i += clusterSize)
		{
			for (uint j = 0; j < rLine; j += clusterSize)
			{
				compMonoMat[k++] = GetThresholdVal(monoMat, i, j, clusterSize, height, width);
			}
			if (rLine < wdth)
				compMonoMat[k++] = GetThresholdVal(monoMat, i, rLine, clusterSize, height, width);
		}
		if (bLine < hght)
		{
			for (uint j = 0; j < rLine; j += clusterSize)
			{
				compMonoMat[k++] = GetThresholdVal(monoMat, bLine, j, clusterSize, height, width);
			}
			if (rLine < wdth)
				compMonoMat[k++] = GetThresholdVal(monoMat, bLine, rLine, clusterSize, height, width);
		}
	}
	if (OnMethodCompleteMsgs) cout << "Matrix compressed!" << endl;
}
void BinarizeMat(const vector<uchar> monoMat, vector<bool> &binMat, uint clusterSize, uint height, uint width)
{
	uint clrng = clusterSize / 2; // Range of cluster
	uint k = 0;
	uchar tval;
	for (uint i = 0; i < height; i++)
	{
		for (uint j = 0; j < width; j++)
		{
			tval = GetThresholdVal(monoMat, max((int)(i - clrng), 0), max((int)(j - clrng), 0), clusterSize, height, width);
			if (monoMat[i*width + j] >= tval)
			{
				binMat[k++] = 0;
				continue;
			}
			binMat[k++] = 1;
		}
	}
	if (OnMethodCompleteMsgs) cout << "Binarization complete!" << endl;
}
long GetObjIntCnt(const vector<bool> binMat)
{
	long objcnt = 0;
	for (int i = 0; i < binMat.size(); i++)
	{
		if (binMat[i]) objcnt++;
	}
	if (OnMethodCompleteMsgs) cout << objcnt << " number of objects returned" << endl;
	return objcnt;
}
void ExtrObjIntCord(const vector<bool> binMat, vector<ObjIntPos> &objsPos, uint height, uint width)
{
	int k = 0;
	for (uint i = 0; i < height; i++)
	{
		for (uint j = 0; j < width; j++)
		{
			if (binMat[i*width + j])
			{
				objsPos[k].row = i;
				objsPos[k++].col = j;
			}
		}
	}
}
void LoadBinarizedMat(vector<uchar> &monoMat, vector<ObjIntPos> &objsPos, vector<bool> &binMat, uint compMltp, uint height, uint width)
{
	//memset(&binMat[0], 0, binMat.size());
	for (int i = 0; i < binMat.size(); i++) binMat[i] = 0;
	if (OnMethodCompleteMsgs) cout << "Background initialization complete." << endl;
	const uint clusterSize = compMltp; // Internal cluster size
	uint clstrRng = clusterSize / 2;
	if (clusterSize % 2 == 0) clstrRng++;
	int k = 0, ind, tval;

	while (k < objsPos.size())
	{
		uint row = objsPos[k].row * compMltp;
		uint col = objsPos[k].col * compMltp;

		for (int i = 0; i < clusterSize; i++)
		{
			for (int j = 0; j < clusterSize; j++)
			{
				tval = GetThresholdVal(monoMat, row - clstrRng + i, col - clstrRng + j, clusterSize, height, width);
				ind = (row + i)*width + col + j;
				if (monoMat[ind] <= tval) binMat[ind] = 1;
			}
		}
		if (OnMethodCompleteMsgs) cout << "Object " << k << "loaded" << endl;
		k++;
	}
	if (OnMethodCompleteMsgs) cout << "Matrix binarized" << endl;
}
void LoadBinMatToTxt(char *destinationPath, const vector<bool> binMat, const uint height, const uint width)
{
	ofstream outputFile;
	outputFile.open(destinationFile);
	char *bufstr = new char[(int)width];
	char *pntrPos = bufstr;
	for (uint i = 0; i < height; i++)
	{
		for (uint j = 0; j < width; j++)
		{
			if (binMat[i*width + j]) *bufstr = '@';
			else *bufstr = ' ';
			bufstr++;
		}
		bufstr = pntrPos;
		outputFile.write(bufstr, width);
		outputFile.put('\n');
	}
	outputFile.close();
	delete[] bufstr;
	delete pntrPos;
	if (OnMethodCompleteMsgs) cout << "Binarized matrix loaded to the output file!" << endl;
}
void FlipUpDown(vector<bool> &binMat, uint height, uint width)
{
	bool top, bot;
	for (int i = 0; i < height / 2; i++)
	{
		for (int j = 0; j < width; j++)
		{
			top = binMat[i*width + j];
			bot = binMat[((height - 1) - i)*width + j];
			if (top != bot)
			{
				binMat[i*width + j] = bot;
				binMat[((height - 1) - i)*width + j] = top;
			}
		}
	}
}
int main()
{
	if (clusterSize % 2 == 0) clusterSize++;
	vector<uchar> monoMat; //Monochrome matrix loaded from a source file.
	LoadMonoMat(sourceFile, monoMat, hght, wdth);
	/*for (int i = 0; i < hght; i++)
	{
		if (i % compression_multiplier == 0) cout << endl;
		for (int j = 0; j < wdth; j++)
		{
			if (j % compression_multiplier == 0) cout << "  ";
			cout << setw(4) << (int)monoMat[i*wdth + j];
		}
		cout << endl;
	}*/
	//If image's dimentions are not multiple of cluster size(C),
	//then we should take this into consideration.
	//Note: The additional right and bottom clusters will have sizes < than the size(C) set on the rest which will result in more sensitive measurements.
	uint cmltp = compression_multiplier;
	uint clsz = clusterSize;
	uint frame = 0; // Used to add the additional clusters.
	if (wdth % cmltp > 0) frame += hght / cmltp;
	if (hght % cmltp > 0)
	{
		if (frame > 0) frame++;
		frame += wdth / cmltp;
	}
	vector<uchar> compMonoMat(((hght / cmltp)*(wdth / cmltp)) + frame); //Array, in which compressed matrix will be stored.
	CompressMat(monoMat, compMonoMat, cmltp, hght, wdth);
	vector<bool> compBinMat(compMonoMat.size());
	uint bhght = hght / cmltp;
	if (hght % cmltp > 0) bhght++;
	uint bwdth = wdth / cmltp;
	if (wdth % cmltp > 0) bwdth++;
	BinarizeMat(compMonoMat, compBinMat, clsz, bhght, bwdth);
	vector<ObjIntPos> objsPos(GetObjIntCnt(compBinMat)); //Stores positions(as indexies) of objects of interest from the compressed matrix
	ExtrObjIntCord(compBinMat, objsPos, bhght, bwdth);
	vector<bool> binMat(monoMat.size());
	LoadBinarizedMat(monoMat, objsPos, binMat, cmltp, hght, wdth);
	FlipUpDown(binMat, hght, wdth);
	LoadBinMatToTxt(destinationFile, binMat, hght, wdth);
	return 0;
}
