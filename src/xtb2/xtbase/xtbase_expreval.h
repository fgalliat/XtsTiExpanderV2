/**
 * XTBase language compiler / interpreter
 * 
 * Xtase - fgalliat @May 2020
 * 
 * Expression Evaluator
 * 
 */

#include "../globals.h"

#include "../xtbase.h"


float XTBaseApp::getNumValue(XTBToken* tk) {
    if ( xtbIsVar(tk) ) {
        Variable* var = xtbGetVar(tk);
        if ( var == NULL ) {
            error("Oups var pbm", 1);
            return -1.0;
        }
        return var->fValue;
    } else if ( xtbIsNum(tk) ) {
        return tk->fValue;
    } else {
        return -1.0;
    }
}

bool XTBaseApp::isNumValue(XTBToken* tk) {
    return ( xtbIsNum(tk) || xtbIsVar(tk) );
}

bool XTBaseApp::isFctCall( XTBToken* tk ) {
    return tk->tkType == XTB_TK_FCT;
}

float XTBaseApp::fctCall( XTBToken** tokens, int nbTokens, int tkNum, int& tkNumRead ) {
    int fctNum = tokens[ tkNum ]->iValue;
    if ( tokens[ tkNum+1 ]->tkType != XTB_TK_PARENTOP ) {
        error("Fct need '('", 1);
        return -1.0;
    }
    int localPointer = tkNum + 2;
    const int FCT_MAX_ARGS = 6;

    int t_size = sizeof(XTBToken*) * FCT_MAX_ARGS;
    XTBToken** args = (XTBToken**)my_malloc( t_size );
    int nbArgs = 0;

    args[nbArgs] = tokens[ tkNum ];
    nbArgs++;

    while( true ) {
        if ( localPointer >= nbTokens ) {
            error("Fct need ')'",2);
            return -1.0;
        }
        if ( tokens[localPointer]->tkType == XTB_TK_PARENTCL ) {
            localPointer++;
            break;
        }
        if ( tokens[localPointer]->tkType == XTB_TK_ARGSEP ) {
            localPointer++;
            continue;
        }
        // read  & store Arg
        int unitLen = 0;
        XTBToken* arg0 = getUnit( tokens, nbTokens, localPointer, unitLen );
        args[nbArgs] = arg0;
        nbArgs++;
        localPointer += unitLen;
    }
    
    float result = (*fct_fun_ptr[ fctNum ])( this, nbArgs, args );    

    freeArgs(args, FCT_MAX_ARGS);
    // my_free(args, t_size);

    tkNumRead = localPointer-tkNum;
    
    return result;
}

// **********

float XTBaseApp::factor(XTBToken** tokens, int nbTokens, int& tkNum)
{
    XTBToken* peeked = tokens[tkNum];

    if ( isNumValue( peeked ) ) {
        float v = getNumValue(peeked);
        tkNum++;
        return v;
    }
    else if ( isFctCall( peeked ) ) {
        int tkNumRead = 0;
        float v = fctCall( tokens, nbTokens, tkNum, tkNumRead );
        if ( tkNumRead == 0 ) {
            error("Calling fct", 1);
            return -1.0;
        }
        tkNum += tkNumRead;
        return v;
    }

    else if (peeked->tkType == XTB_TK_PARENTOP)
    {
        tkNum++; // '('
        float result = expression(tokens, nbTokens, tkNum);
        tkNum++; // ')'
        return result;
    }
    else if (peeked->tkType == XTB_TK_OPR && peeked->iValue == (int)'-')
    {
        // ex. -3.14
        tkNum++;
        return -factor(tokens, nbTokens, tkNum);
    }
    return 0.0; // error
}

float XTBaseApp::term(XTBToken** tokens, int nbTokens, int& tkNum)
{
    XTBToken* peeked = tokens[tkNum];
    char peekedOp = (char)peeked->iValue;

    float result = factor(tokens, nbTokens, tkNum);
    if ( tkNum >= nbTokens ) { return result; }
    peeked = tokens[tkNum];
    peekedOp = (char)peeked->iValue;

    // MOD '%' is like a DIV '/'
    while ( peeked->tkType == XTB_TK_OPR && (peekedOp == '*' || peekedOp == '/' || peekedOp == '%') ) {
        tkNum++;
        
        if (peekedOp == '*') {
            result *= factor(tokens, nbTokens, tkNum);
        }
        else if (peekedOp == '%') {
            int tmp = (int)result;
            int tmp2 = (int)factor(tokens, nbTokens, tkNum);

            if ( tmp2 == 0) {
                error("Mod by 0", 1);
                return 0.0;
            }

            tmp %= tmp2;
            result = (float)tmp;
        }
        else {
            float tmp2 = factor(tokens, nbTokens, tkNum);
            if ( tmp2 == 0.0) {
                error("Div by 0", 1);
                return 0.0;
            }
            result /= tmp2;
        }

        if ( tkNum >= nbTokens ) { break; }
        peeked = tokens[tkNum];
        peekedOp = (char)peeked->iValue;
    }
    return result;
}

float XTBaseApp::arith_expression(XTBToken** tokens, int nbTokens, int& tkNum)
{
    XTBToken* peeked = tokens[tkNum];
    char peekedOp = (char)peeked->iValue;

    float result = term(tokens, nbTokens, tkNum);
    if ( tkNum >= nbTokens ) { return result; }
    peeked = tokens[tkNum];
    peekedOp = (char)peeked->iValue;

    while ( peeked->tkType == XTB_TK_OPR && (peekedOp == '+' || peekedOp == '-') ) {
        tkNum++;
        

        if (peekedOp == '+') {
            result += term(tokens, nbTokens, tkNum);
        }
        else {
            result -= term(tokens, nbTokens, tkNum);
        }

        if ( tkNum >= nbTokens ) { break; }
        peeked = tokens[tkNum];
        peekedOp = (char)peeked->iValue;
    }
    return result;
}

// =============
// =============


bool XTBaseApp::isConditionOperator(XTBToken* peeked) {
    if (peeked->tkType == XTB_TK_OPR_BOOL && (
        peeked->iValue == BOP_EQ ||
        peeked->iValue == BOP_NEQ || peeked->iValue == BOP_NEQ2 ||
        peeked->iValue == BOP_GT || peeked->iValue == BOP_LT ||
        peeked->iValue == BOP_GTE || peeked->iValue == BOP_LTE
    ) ) {
        return true;
    }
    return false;
}

// assumes that compOp is really a ConditionOperator
bool XTBaseApp::testCondition(float value, XTBToken* compOp, float value2) {
    int opType= compOp->iValue;

    if ( opType == BOP_EQ ) {
        return value == value2;
    } else if ( opType == BOP_NEQ || opType == BOP_NEQ2 ) {
        return value != value2;
    } else if( opType == BOP_GT ) {
        return value > value2;
    } else if( opType == BOP_LT ) {
        return value < value2;
    } else if( opType == BOP_GTE ) {
        return value >= value2;
    } else if( opType == BOP_LTE ) {
        return value <= value2;
    }

    return false;
}


float XTBaseApp::bool_cond(XTBToken** tokens, int nbTokens, int& tkNum)
{
    int curTkNum = tkNum;
    float result = arith_expression( tokens, nbTokens, tkNum );
    if ( tkNum >= nbTokens ) { return result; }

    XTBToken* peeked = tokens[tkNum];
    if (isConditionOperator( peeked ) )
    {
        tkNum++;
        result = testCondition( result, peeked, arith_expression(tokens, nbTokens, tkNum) ) ? 1.0 : 0.0;
        return result;
    }

    return result;
}



float XTBaseApp::bool_factor(XTBToken** tokens, int nbTokens, int& tkNum)
{
    XTBToken* peeked = tokens[tkNum];

    if (peeked->tkType == XTB_TK_PARENTOP)
    {
        tkNum++; // '('
        float result = expression(tokens, nbTokens, tkNum);
        tkNum++; // ')'
        return result;
    }
    else if (peeked->tkType == XTB_TK_OPR_BOOL && peeked->iValue == (int)BOP_NOT)
    {
        // ex. !true
        tkNum++;
        return ! (bool_factor(tokens, nbTokens, tkNum) != 0.0 );
    } 
    else {
        //return arith_expression( tokens, tkNum );
        return bool_cond( tokens, nbTokens, tkNum );
    }
    return 0.0; // error
}

float XTBaseApp::bool_term(XTBToken** tokens, int nbTokens, int& tkNum)
{
    XTBToken* peeked = tokens[tkNum];
    int peekedOp = peeked->iValue;

    float result = bool_factor(tokens, nbTokens, tkNum);
    if ( tkNum >= nbTokens ) { return result; }
    peeked = tokens[tkNum];
    peekedOp = (char)peeked->iValue;

    bool init = false;
    bool cond = result != 0.0;
    while ( peeked->tkType == XTB_TK_OPR_BOOL && (peekedOp == BOP_OR) ) {
        tkNum++;
        
        cond |= bool_factor(tokens, nbTokens, tkNum) != 0.0;
        init = true;

        if ( tkNum >= nbTokens ) { break; }
        peeked = tokens[tkNum];
        peekedOp = peeked->iValue;
    }
    return init ? (cond ? 1.0 : 0.0) : result;
}


float XTBaseApp::expression(XTBToken** tokens, int nbTokens, int& tkNum)
{
    XTBToken* peeked = tokens[tkNum];
    int peekedOp = peeked->iValue;

    float result = bool_term(tokens, nbTokens, tkNum);
    if ( tkNum >= nbTokens ) { return result; }
    peeked = tokens[tkNum];
    peekedOp = (char)peeked->iValue;

    bool init = false;
    bool cond = result != 0.0;
    while ( peeked->tkType == XTB_TK_OPR_BOOL && (peekedOp == BOP_AND) ) {
        tkNum++;

        cond &= bool_term(tokens, nbTokens, tkNum) != 0.0;
        init = true;

        if ( tkNum >= nbTokens ) { break; }
        peeked = tokens[tkNum];
        peekedOp = peeked->iValue;
    }
    return init ? (cond ? 1.0 : 0.0) : result;
}


// **********

XTBToken* XTBaseApp::getUnit( XTBToken** tokens, int nbTokens, int tkNum, int &unitLen ) {
    if ( tkNum >= nbTokens ) { unitLen = 0; return NULL; }
    XTBToken* tk = tokens[tkNum];
    XTBToken* nextTk = ( tkNum+1 >= nbTokens ) ? NULL : tokens[tkNum+1];

    if ( tk->tkType == XTB_TK_STR ) {
        // can't concat strings as "T"+"O"
        // so will be unit
        unitLen = 1;
        return tk;
    } else if ( tk->tkType == XTB_TK_NUM || tk->tkType == XTB_TK_VAR ) {

        // may be a num/bool complex expr
        if ( nextTk != NULL && (nextTk->tkType == XTB_TK_OPR || nextTk->tkType == XTB_TK_OPR_BOOL) ) {
            int cTkNum = tkNum;
            float val = expression(tokens, nbTokens, cTkNum);
            unitLen = cTkNum - tkNum;

            tk = newToken(); // FIXME
            tk->toBeDestroyed = true;
            tk->tkType = XTB_TK_NUM;
            tk->fValue = val;

            return tk;
        } else {
            // just 1 num / var
            // if (tk->tkType == TK_VAR) {
            unitLen = 1;
            return tk;
        }
    } else if ( tk->tkType == XTB_TK_FCT ) {
        // will be a num complex expr // "cos(3+1)"

        int cTkNum = tkNum;
        float val = expression(tokens, nbTokens, cTkNum);
        unitLen = cTkNum - tkNum;

        tk = newToken(); // FIXME
        tk->toBeDestroyed = true;
        tk->tkType = XTB_TK_NUM;
        tk->fValue = val;

        return tk;
    } else if ( tk->tkType == XTB_TK_PARENTOP ) {
        // will be a num complex expr // "(3+1)"

        int cTkNum = tkNum;
        float val = expression(tokens, nbTokens, cTkNum);
        unitLen = cTkNum - tkNum;

        tk = newToken(); // FIXME
        tk->toBeDestroyed = true;
        tk->tkType = XTB_TK_NUM;
        tk->fValue = val;

        return tk;
    } else if ( tk->tkType == XTB_TK_OPR ) {
        // will be a num complex expr // "-10"

        int cTkNum = tkNum;
        float val = expression(tokens, nbTokens, cTkNum);
        unitLen = cTkNum - tkNum;

        tk = newToken(); // FIXME
        tk->toBeDestroyed = true;
        tk->tkType = XTB_TK_NUM;
        tk->fValue = val;

        return tk;
    } else if ( tk->tkType == XTB_TK_FCT ) {
        // _disp("fct");
        // will be a complex expr
        unitLen = 10;
    } else if ( tk->tkType == XTB_TK_EOL ) {
        unitLen = 1;
        return tk;
    } else if ( tk->tkType == XTB_TK_EOP ) {
        unitLen = 1;
        return tk;
    }
    print("other :");
    print(tk->tkType);
    print(" ");
    print(xtb_token_type[ tk->tkType ] );
    println("");
    
    unitLen = 0;
    return NULL;
}