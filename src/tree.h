// tree.h - Copyright (C) 2004 Reed A. Cartwright (all rights reserved)

#ifndef DAWG_TREE_H
#define DAWG_TREE_H

#include "dawg.h"
#include "indel.h"
#include "matrix.h"


// A class used to represent a node in a Newick tree
class NewickNode {
public:
	NewickNode(NewickNode* p, const char *cs, double d);
	std::string m_ssLabel;
	double m_dLen;
	std::auto_ptr<NewickNode> m_pSib;
	std::auto_ptr<NewickNode> m_pSub;

protected:
	void MakeName();
};

// A class to represent nucleotides
class Nucleotide
{
public:
	// First two bits specify base
	// Second two bits specify type
	unsigned char    m_ucNuc;
	double m_dRate; // 0.0 means invarant

	Nucleotide() : m_ucNuc(0xF), m_dRate(1.0) { }
	Nucleotide(unsigned char nuc, double rate) : m_ucNuc(nuc), m_dRate(rate) { }

	static const unsigned char MaskBase		= 0x3; // 0011
	static const unsigned char MaskType		= 0xC; // 1100
	static const unsigned char MaskDel		= 0x8; // 1000
	static const unsigned char MaskIns		= 0x4; // 0100
	static const unsigned char TypeRoot		= 0x0; // 0000
	static const unsigned char TypeIns		= 0x4; // 0100
	static const unsigned char TypeDel		= 0x8; // 1000
	static const unsigned char TypeDelIns	= 0xC; // 1100
	
	inline unsigned char GetBase() const { return m_ucNuc & MaskBase; }
	inline unsigned char GetType() const { return m_ucNuc & MaskType; }
	inline void SetBase(unsigned char uc) { m_ucNuc =  (uc & MaskBase) | (m_ucNuc & MaskType); }
	inline void SetType(unsigned char uc) { m_ucNuc =  (uc & MaskType) | (m_ucNuc & MaskBase); }
	inline void SetNuc(unsigned char ucB, unsigned char ucT)
		{ m_ucNuc =  (ucB & MaskBase) | (ucT & MaskType); }
	inline bool IsType(unsigned char uc) const { return (GetType() == uc); }
	inline bool IsDeletion() const { return ((m_ucNuc & MaskDel) == MaskDel); }
	inline bool IsInsertion() const { return ((m_ucNuc & MaskIns) == MaskIns); }

	bool FromChar(char ch);
	char ToChar() const;

};

// A class that represent a sequence of nucleotides
class Sequence : public std::vector<Nucleotide>
{
public:
	typedef std::vector<Nucleotide> Base;
	Sequence() : m_uLength(0) { }
	explicit Sequence(size_type uSize) : Base(uSize, Nucleotide(0xF, -1.0))
	{
		m_uLength = uSize;
	}
	size_type SeqLength() const { return m_uLength; }
	
	// find the uPos-th true nucleotide (skips gaps)
	const_iterator SeqPos(size_type uPos) const;
	iterator SeqPos(size_type uPos);

	size_type Insertion(iterator itPos, const_iterator itBegin, const_iterator itEnd);
	size_type Deletion(iterator itBegin, Base::size_type uSize);

	void Append(const Sequence &seq);

	void ToString(std::string &ss) const;

private:
	size_type m_uLength;
};


// The recombinant tree data structure
class Tree
{
public:

	// A node in the tree
	class Node
	{
	public:
		typedef std::map<std::string, Tree::Node> Map;
		typedef Map::iterator Handle;
		
		std::vector<Sequence> m_vSections;
		std::vector<Handle> m_vAncestors;
		std::map<Handle, double> m_mBranchLens;
		std::string m_ssName;
		bool m_bTouched;

		Node() : m_bTouched(false) { }
		void Flatten(Sequence& seq) const;
		Sequence::size_type SeqLength() const;

		typedef std::pair<std::vector<Sequence>::iterator, Sequence::iterator> iterator;
		typedef std::pair<std::vector<Sequence>::const_iterator, Sequence::const_iterator> const_iterator;
	
		// find the uPos-th nucleotide in the node
		// skips gaps and recognizes different sections
		iterator SeqPos(Sequence::size_type uPos);
		const_iterator SeqPos(Sequence::size_type uPos) const;
	};

	typedef std::map<std::string, std::string> Alignment;
	
	// Setup the model of evolution
	bool SetupEvolution(double pFreqs[], double pSubs[],
		const IndelModel::Params& rIns, const IndelModel::Params& rDel,
		unsigned int uWidth, const std::vector<double> &vdGamma,
		const std::vector<double> &vdIota, const std::vector<double> &vdScale, double dTreeScale);
	
	// Setup the root node
	bool SetupRoot(const std::vector<std::string> &vSeqs, const std::vector<unsigned int> &vData,
		const std::vector<std::vector<double> > &vRates);
	
	// Draw a random relative rate of substitution from the evolutionary parameters
	double RandomRate(Sequence::size_type uPos) const;
	// Draw a random base from the evolutionary parameters
	unsigned char RandomBase() const;
	// Draw a random nucleotide (base and rate)
	Nucleotide RandomNucleotide(Sequence::size_type uPos) const
		{ return Nucleotide(RandomBase(), RandomRate(uPos)); }

	Tree() : m_nSec(0), m_uWidth(1) {}
	
	// Trim a length to be compatible with the block width
	inline unsigned int BlockTrim(unsigned int u) { return u - u%m_uWidth; }
	
	// Evolve the tree
	void Evolve();

	// Add a recombination section to the tree
	void ProcessTree(NewickNode* pNode);
	
	const Node::Map& GetMap() const { return m_map; }
	
	// Align sequences from the tree
	void Align(Alignment &aln) const;

protected:
	void ProcessNewickNode(NewickNode* pNode, Node::Handle hAnc);
	void Evolve(Node &rNode, double dTime);
	void Evolve(Node &rNode);

private:
	int m_nSec;
	std::vector< Sequence > m_vDNASeqs;
	Node::Map m_map;
	std::vector<std::string> m_vTips;

	unsigned int m_uWidth;
	std::vector<double> m_vdScale;
	std::vector<double> m_vdGamma;
	std::vector<double> m_vdIota;

	double m_dTreeScale;

	double m_dOldTime;
	double m_dFreqs[4];
	double m_dNucCumFreqs[4];
	Matrix44 m_matSubst;
	Matrix44 m_matV;
	Matrix44 m_matU;
	Matrix44 m_matQ;
	Matrix44 m_matR;
	Vector4  m_vecL;

	std::auto_ptr<IndelModel> m_pInsertionModel;
	std::auto_ptr<IndelModel> m_pDeletionModel;
	double m_dLambdaIns;
	double m_dLambdaDel;
	LinearFunc m_funcRateIns;
	LinearFunc m_funcRateSum;
};

bool SaveAlignment(std::ostream &rFile, const Tree::Alignment& aln, unsigned int uFlags);

namespace std {

inline bool operator < (const Tree::Node::Handle & A, const Tree::Node::Handle & B)
{
	return &*A < &*B;
}

}

#endif //DAWG_TREE_H

