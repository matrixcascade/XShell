#pragma  once 


#include "Cube_Lexer.h"
#include <regex>

#include <list>
#define  CUBE_GRAMMAR_TYPE_MAX_LIST		32
#define  CUBE_GRAMMAR_TYPE_MAX_LEMSIZE	128

#define  GRAMMAR_TYPE_ANY       0

///////////////////////////////////////////////////////////////////
//Layer 0
#define  GRAMMAR_TYPE_ANY		     0
#define  GRAMMAR_TOKEN				 1
#define  GRAMMAR_SPACER				 2
#define  GRAMMAR_DELIMITER			 3
#define  GRAMMAR_CONATINER           4
#define  GRAMMAR_NEWLINE	         5
#define  GRAMMAR_PARAMETER           6
#define  GRAMMAR_UNDEFINED			 6
#define  GRAMMAR_SENTENCE_UNKNOW			 ((unsigned int)(-2))
#define  GRAMMAR_SENTENCE_END				 ((unsigned int)(-1))


struct CubeBlockType
{
	unsigned int	BlockTypeTree[CUBE_GRAMMAR_TYPE_MAX_LIST];
	char			Mnemonic[CUBE_GRAMMAR_TYPE_MAX_LEMSIZE];
	regex			pattern;
	BOOL            bRegex;

	CubeBlockType(char *Mne,int Layer,...)
	{
		va_list arg_ptr; 
		memset(BlockTypeTree,0,sizeof(BlockTypeTree));
		memset(Mnemonic,0,sizeof(Mnemonic));

		bRegex=FALSE;
		if(Mne)
		{
		#ifdef _DEBUG
			if (strlen(Mne)>=CUBE_GRAMMAR_TYPE_MAX_LEMSIZE)
			{
				assert(0);//Mnemonic is too long.
			}
		#endif}
		strcpy(Mnemonic,Mne);

		}
		va_start(arg_ptr,Layer); 
		for(int Offset=0;Offset<Layer;Offset++)
		BlockTypeTree[Offset]=va_arg(arg_ptr, int); 
		va_end(arg_ptr);
	}
	CubeBlockType()
	{
		memset(BlockTypeTree,0,CUBE_GRAMMAR_TYPE_MAX_LIST);
		memset(Mnemonic,0,sizeof(Mnemonic));
		bRegex=FALSE;
	}

	~CubeBlockType()
	{
	}
	void AsRegex(BOOL Regex=TRUE)
	{
		if(Regex)
		{
			regex tpattern(Mnemonic,regex_constants::extended);
			pattern=tpattern;
		}
		bRegex=Regex;
	}


	BOOL IsCharNumeric(char ch)
	{
		return ch>='0'&&ch<='9';
	}

	BOOL IsNumeric()
	{
		BOOL Dot=FALSE;
		if (Mnemonic==NULL)
		{
			return FALSE;
		}

		if (strlen(Mnemonic)==0)
		{
			return FALSE;
		}
		unsigned int CurrentCharIndex;

		if (!IsCharNumeric(Mnemonic[0])&&!(Mnemonic[0]==('-')))
		{
			if(Mnemonic[0]!='.')
			return FALSE;
		}
		for (CurrentCharIndex=1;
			CurrentCharIndex<strlen(Mnemonic);
			CurrentCharIndex++)
		{
			if (!IsCharNumeric(Mnemonic[CurrentCharIndex]))
			{
				if (Mnemonic[CurrentCharIndex]=='#')
				{
					return TRUE;
				}
				if (Mnemonic[CurrentCharIndex]=='e'&&(Mnemonic[CurrentCharIndex+1]=='+'||Mnemonic[CurrentCharIndex+1]=='-'))
				{
					CurrentCharIndex++;
					continue;
				}

				if (Mnemonic[CurrentCharIndex]=='.')
				{
					if (Dot)
					{
						return FALSE;
					}
					else
					{
						Dot=TRUE;
						continue;
					}
				}
				return FALSE;
			}
		}
		return TRUE;
	}

	void Set(char *Mne,int Layer,...)
	{
		va_list arg_ptr; 
		memset(BlockTypeTree,0,sizeof(BlockTypeTree));
		memset(Mnemonic,0,sizeof(Mnemonic));
#ifdef _DEBUG
		if (strlen(Mne)>=CUBE_GRAMMAR_TYPE_MAX_LEMSIZE)
		{
			assert(0);//Mnemonic too long.
		}
#endif
		if(Mne)
			strcpy(Mnemonic,Mne);

		va_start(arg_ptr,Layer); 
		for(int Offset=0;Offset<Layer;Offset++)
			BlockTypeTree[Offset]=va_arg(arg_ptr, int); 
		va_end(arg_ptr); 
	}
	//equal mnemonic
	BOOL operator==(char *Mne)
	{
		return IsMemonicMatch(Mne);
	}
	//equal
	BOOL operator==(CubeBlockType& Token)
	{
		if(Mnemonic[0]!=0)
		return (memcmp(Token.BlockTypeTree,BlockTypeTree,CUBE_GRAMMAR_TYPE_MAX_LIST)==0)&&(strcmp(Mnemonic,Token.Mnemonic)==0);
		else
		return (memcmp(Token.BlockTypeTree,BlockTypeTree,CUBE_GRAMMAR_TYPE_MAX_LIST)==0);
	}
	//unequal
	BOOL operator!=(CubeBlockType& Token)
	{
		return !(*this==Token);
	}
	//Included
	BOOL operator>>(CubeBlockType &Token)
	{
		for (int i=0;i<CUBE_GRAMMAR_TYPE_MAX_LIST;i++)
		{
			if (Token.BlockTypeTree[i]==0)
			{
				return TRUE;
			}
			if (BlockTypeTree[i]!=Token.BlockTypeTree[i])
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	//Include
	BOOL  operator<<(CubeBlockType &Token)
	{
		for (int i=0;i<CUBE_GRAMMAR_TYPE_MAX_LIST;i++)
		{
			if (BlockTypeTree[i]==0)
			{
				return TRUE;
			}
			if (BlockTypeTree[i]!=Token.BlockTypeTree[i])
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	//Is Mnemonic match
	BOOL IsMemonicMatch(char *Str)
	{
		if (!bRegex)
		{
			return strcmp(Mnemonic,Str)==0;
		}
		else
		{
			match_results<string::const_iterator> result;
			string s(Str);
			return regex_match(s,result,pattern);
		}
	}
};


struct CubeGrammarSentence
{
	CubeGrammarSentence()
	{
		
	}
	~CubeGrammarSentence()
	{
		Reset();
	}

	void Reset()
	{
		m_vBlocks.clear();
	}
	void add(CubeBlockType &Token)
	{
		m_vBlocks.push_back(Token);
	}

	unsigned int GetBlocksCount()
	{
		return m_vBlocks.size();
	}

	char *GetBlockString(unsigned int i)
	{
		if (i>=m_vBlocks.size())
		{
			return NULL;
		}
		return m_vBlocks[i].Mnemonic;
	}

	vector<CubeBlockType> m_vBlocks;
};

using namespace std;

class CubeGrammar
{
public: 
	CubeGrammar()
	{
		Free();
	}
	~CubeGrammar()
	{
		Free();
	}

	void SetLexer(CubeLexer *Lexer)
	{
		if(Lexer)
		m_Lexer=Lexer;
	}

	void RegisterBlockType(CubeBlockType Type)
	{
		for (unsigned int i=0;i<m_vRegBlock.size();i++)
		{
			if (m_vRegBlock[i]==Type)
			{
				return;
			}
		}
		m_vRegBlock.push_back(Type);
		return;
	}


	void Free()
	{
		m_Lexer=NULL;
		m_vRegBlock.clear();
		m_vRegGammarSentence.clear();
		m_vDiscard.clear();
	}

	CubeBlockType  GetBlockByMne(char *Mne)
	{
		for (unsigned int i=0;i<m_vRegBlock.size();i++)
		{
			if (m_vRegBlock[i]==Mne)
			{
				return m_vRegBlock[i];
			}
		}
		//Undefined
		return CubeBlockType(Mne,1,GRAMMAR_UNDEFINED);
	}

	BOOL MatchAt(CubeGrammarSentence &ResStream,CubeGrammarSentence &DesStream,int index)
	{
		return ResStream.m_vBlocks[index]>>DesStream.m_vBlocks[index];
	}

	BOOL IsDiscard(CubeBlockType &Block)
	{
		for (unsigned int i=0;i<m_vDiscard.size();i++)
		{
			if (Block>>m_vDiscard[i])
			{
				return TRUE;
			}
		}
		return FALSE;
	}

	unsigned int RegisterSentence(CubeGrammarSentence &Str)
	{
		m_vRegGammarSentence.push_back(Str);
		return m_vRegGammarSentence.size()-1;
	}

	unsigned int GetNextInstr(CubeGrammarSentence &Stream)
	{
		m_vIst.clear();
		Stream.Reset();
		for (unsigned int i=0;i<m_vRegGammarSentence.size();i++)
		{
			m_vIst.push_back(i);
		}
		list<unsigned int>::iterator it;
		int Offset=0;

		while(!m_vIst.empty())
		{
			CubeBlockType Tok;

			switch(m_Lexer->GetNextLexeme())
			{
			case CUBE_LEXER_LEXEME_TYPE_CONATINER:
				Tok=GetBlockByMne(m_Lexer->GetLexemeString());
				if(Tok==CubeBlockType(NULL,1,GRAMMAR_UNDEFINED))
				Tok.Set(m_Lexer->GetLexemeString(),1,GRAMMAR_CONATINER);
				break;
			case CUBE_LEXER_LEXEME_TYPE_DELIMITER:
				Tok=GetBlockByMne(m_Lexer->GetLexemeString());
				if(Tok==CubeBlockType(NULL,1,GRAMMAR_UNDEFINED))
				Tok.Set(m_Lexer->GetLexemeString(),1,GRAMMAR_DELIMITER);
				break;
			case CUBE_LEXER_LEXEME_TYPE_NEWLINE:
				Tok.Set(m_Lexer->GetLexemeString(),1,GRAMMAR_NEWLINE);
				break;
			case CUBE_LEXER_LEXEME_TYPE_SPACER:
				Tok.Set(m_Lexer->GetLexemeString(),1,GRAMMAR_SPACER);
				break;
			case CUBE_LEXER_LEXEME_TYPE_TOKEN:
				Tok=GetBlockByMne(m_Lexer->GetLexemeString());
				break;
			case CUBE_LEXER_LEXEME_TYPE_END:
				//Set "END" as new line directly
				return (unsigned int)(-1);
				break;
			}

			//Discard type
			if (IsDiscard(Tok))
			{
			continue;
			}

			strcpy(Tok.Mnemonic,m_Lexer->GetLexemeString());

			it = m_vIst.begin();
			while (TRUE)
			{
				if (!(Tok>>m_vRegGammarSentence[*it].m_vBlocks[Offset]))
				{
					list<unsigned int>::iterator rm;
					rm=it;
					it++;
					m_vIst.erase(rm);
				}
				else
				{
					if(Offset==(m_vRegGammarSentence[*it].GetBlocksCount()-1))
					{
						Stream.add(Tok);
						return *it;
					}
					it++;
				}

				if (it==m_vIst.end())
				{
					break;
				}
			}
			Stream.add(Tok);
			Offset++;
		}

		return GRAMMAR_SENTENCE_UNKNOW;

	}

	void RegisterDiscard(CubeBlockType &Type)
	{
		m_vDiscard.push_back(Type);
	}

private: 
	CubeLexer					*m_Lexer;
	vector<CubeBlockType>		m_vRegBlock;
	vector<CubeBlockType>		m_vDiscard;
	vector<CubeGrammarSentence>	m_vRegGammarSentence;
	
	list<unsigned int>	m_vIst;

	
};