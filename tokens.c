#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#include "parser.h"


typedef struct VXMToken VXMToken;
typedef enum VXMTokenType VXMTokenType;


enum
VXMTokenType
{
    VXMTokenNumber,
    VXMTokenString,
    VXMTokenBool,
    VXMTokenArrayStart,
    VXMTokenArraySeperator,
    VXMTokenArrayEnd,
    VXMTokenIdentifier,

    VXMTokenIf,
    VXMTokenElse,
    VXMTokenWhile,
    VXMTokenForEach,
    VXMTokenFunction,

    VXMTokenPlus,
    VXMTokenPlusEq,
    VXMTokenMinus,
    VXMTokenMinusEq,
    VXMTokenMul,
    VXMTokenMulEq,
    VXMTokenDiv,
    VXMTokenDivEq,
    VXMTokenMod,
    VXMTokenModEq,
    VXMTokenAssign,
    VXMTokenEq,     /* == */
    VXMTokenNeq,    /* != */
    VXMTokenLt,     /* < */
    VXMTokenLte,    /* <= */
    VXMTokenGt,     /* > */
    VXMTokenGte,    /* >= */
    VXMTokenAnd,
    VXMTokenOr,
    VXMTokenNot,

    VXMTokenBitAnd,
    VXMTokenBitOr,
    VXMTokenBitXor,


    VXMTokenOpen,
    VXMTokenClose,
    VXMTokenBlockStart,
    VXMTokenBlockEnd,

    VXMTokenComment,

    VXMTokenEOL,
    VXMTokenEOF,

    VXMTokenError,

    // hidden
    VXMTokenJmp,
};

struct
VXMToken
{
    VXMTokenType type;
    char *text;
    long line;
};

struct
VXMKeyword
{
    char *word;
    VXMTokenType type;
    size_t length;
};

struct VXMKeyword keywords[] = 
{
    {"foreach", VXMTokenForEach, 7},
    {"while", VXMTokenWhile, 5},
    {"false", VXMTokenBool, 5},
    {"true", VXMTokenBool, 4},
    {"else", VXMTokenElse, 4},
    {"if", VXMTokenIf, 2},
};

VXMToken
VXMTokenCreate(VXMTokenType type, char *text, long line)
{   return (VXMToken){ .type = type, .text = text, .line = line };
}

VXMToken
VXMTokenParseString(char *line, long line_num, long *end_at)
{
    long i = 0;
    char *text;

    if(line[i] == '"')
    {   ++i;
    }
    else
    {   
        *end_at = i;
        return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    long start = i;

    while(line[i] != '"' && line[i] != '\0')
    {   ++i;
    }

    if(line[i] == '\0')
    {   
        *end_at = i;
        return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    long length = i - start;;

    text = malloc((length + 1) * sizeof(char));

    if(!text)
    {   
        *end_at = i + 1;
        return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    memcpy(text, line + start, length);

    text[length] = '\0';

    *end_at = i + 1;

    return VXMTokenCreate(VXMTokenString, text, line_num);
}

VXMToken
VXMTokenParserNumber(char *line, long line_num, long *end_at)
{
    long i = 0;
    char *text;

    long start = i;
    bool dotAlreadyMade = false;

    while(isdigit(line[i]) || line[i] == '.')
    {   
        if(line[i] == '.')
        {
            if(dotAlreadyMade)
            {   return VXMTokenCreate(VXMTokenError, NULL, line_num);
            }
            dotAlreadyMade = true;
        }

        ++i;
    }

    long length = i - start;

    if(length == 0 || (length == 1 && dotAlreadyMade))
    {   return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    text = malloc((length + 1) * sizeof(char));

    if(!text)
    {   
        *end_at = i;
        return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    memcpy(text, line + start, length);

    text[length] = '\0';

    *end_at = i;

    return VXMTokenCreate(VXMTokenNumber, text, line_num);
}

VXMToken
VXMTokenParserVariable(char *line, long line_num, long *end_at)
{
    long i = 0;
    char *text;

    long start = i;

    while(isalnum(line[i]) || line[i] == '_')
    {   ++i;
    }

    if(i == start)
    {
        *end_at = i;
        return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    long length = i - start;

    text = malloc((length + 1) * sizeof(char));

    if(!text)
    {   
        *end_at = i;
        return VXMTokenCreate(VXMTokenError, NULL, line_num);
    }

    memcpy(text, line + start, length);

    text[length] = '\0';

    *end_at = i;

    return VXMTokenCreate(VXMTokenIdentifier, text, line_num);
}

static bool
STRING_ATLEAST(char *str, long length)
{
    long i;

    for(i = 0; i < length; ++i)
    {
        if(str[i] == '\0')
        {   return false;
        }
    }

    return true;
}

VXMToken
VXMTokenParsePassive(char *line, long line_num, long *end_at)
{
    long start = 0;
    char c = line[start];

    if(isalpha(c))
    {
        long i;

        for(i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
        {
            struct VXMKeyword kw = keywords[i];

            if(STRING_ATLEAST(line, kw.length) && strncmp(line, kw.word, kw.length) == 0)
            {
                if(isspace(line[kw.length]) 
                    ||
                    ((kw.type == VXMTokenForEach || kw.type == VXMTokenIf || kw.type == VXMTokenWhile) 
                     && line[kw.length] == '(')
                    )
                {
                    *end_at = kw.length;
                    return VXMTokenCreate(kw.type, kw.word, line_num);
                }
            }
        }

        return VXMTokenParserVariable(line, line_num, end_at);
    }
    else if(c == '_')
    {   return VXMTokenParserVariable(line, line_num, end_at);
    }

    if(isdigit(c) || c == '.')
    {   return VXMTokenParserNumber(line, line_num, end_at);
    }
}

VXMToken *
VXMTokenizeLine(char *line, long line_num)
{
    long end = 0;
    char c;

    long i = 0;
    long tokens_length = 0;
    bool firstPassThrough = false;
    enum { MAX_TOKENS = LONG_MAX };

    VXMToken *tokens = NULL;

LOOP:
    while(line[i] != '\0')
    {
        VXMToken output;
        bool push = true;

        c = line[i];
        end = 1;

        switch(c)
        {
            // string.
            case '"':
                output = VXMTokenParseString(line + i, line_num, &end);
                break;
            case '+':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenPlusEq, "+=", line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenPlus, "+" , line_num);
                }
                break;
            case '-':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenMinus, "-=" , line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenMinus, "-" , line_num);
                }
                
                break;
            case '*':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenMul, "*=" , line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenMul, "*" , line_num);
                }
                break;
            case '/':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenDiv, "/=" , line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenDiv, "/" , line_num);
                }
                break;
            case '%':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenMod, "%=" , line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenMod, "%" , line_num);
                }
                
                break;
            case '&':
                output = VXMTokenCreate(VXMTokenBitAnd, "&" , line_num);
                break;
            case '^':
                output = VXMTokenCreate(VXMTokenBitXor, "^" , line_num);
                break;
            case '|':
                output = VXMTokenCreate(VXMTokenBitOr, "|" , line_num);
                break;

            case '=':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenEq, "==", line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenAssign, "=", line_num);
                }
                break;
            case '!':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenNeq, "!=", line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenNot, "!", line_num);
                }
                break;
            case '>':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenGte, ">=", line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenGt, ">", line_num);
                }
                break;
            case '<':
                if(line[i + 1] == '=')
                {   
                    output = VXMTokenCreate(VXMTokenLte, "<=", line_num);
                    ++end;
                }
                else
                {   output = VXMTokenCreate(VXMTokenLt, "<", line_num);
                }
                break;
            case '#':
                output = VXMTokenCreate(VXMTokenComment, "#", line_num);
                break;
            // array
            case '[':
                output = VXMTokenCreate(VXMTokenArrayStart, "[", line_num);
                break;
            case ',':
                output  = VXMTokenCreate(VXMTokenArraySeperator, ",", line_num);
                break;
            case ']':
                output = VXMTokenCreate(VXMTokenArrayEnd, "]", line_num);
                break;
            case '(':
                output = VXMTokenCreate(VXMTokenOpen, "(", line_num);
                break;
            case ')':
                output = VXMTokenCreate(VXMTokenClose, ")", line_num);
                break;
            default:
                VXMTokenParsePassive(line + i, line_num, &end);
                break;
        }

        i += end;

        if(push)
        {
            if(output.type == VXMTokenError)
            {   
                if(tokens)
                {   free(tokens);
                }

                return NULL;
            }

            if(!firstPassThrough)
            {   
                tokens[tokens_length] = output;
                ++tokens_length;
            }

            if(output.type == VXMTokenComment)
            {   return tokens;
            }

            continue;
        }

        if(isspace(c))
        {   
            ++i;
            continue;;
        }
    }

    if(firstPassThrough)
    {
        firstPassThrough = false;
        tokens = malloc(tokens_length * sizeof(*tokens));

        if(!tokens)
        {   return NULL;
        }

        goto LOOP;
    }

    return tokens;
}













