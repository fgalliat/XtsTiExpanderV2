/**
 * XTBase (Xtase May2020)
 * 
 * Statements manipulation
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"


bool xtbIsAspecificStatement(Token* tk) {
    if ( tk == NULL ) { return true; }
    if ( tk->tkType == XTB_TK_EOP ) { return true; }

    if ( tk->tkType == XTB_TK_KEYW ) { 
        if ( tk->iValue == XTB_KW_LBL || tk->iValue == XTB_KW_GOSUB || tk->iValue == XTB_KW_GOTO || 
             tk->iValue == XTB_KW_FOR || tk->iValue == XTB_KW_IF ) { 
            return true; 
        }

        if ( tk->iValue == XTB_KW_END || tk->iValue == XTB_KW_RETURN ) { 
            return true; 
        }
    }

    return false;
}

bool xtbIsThenToken(Token* tk) {
    return tk != NULL && tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_THEN;
}

bool xtbIsThenStatement(Statement* stmt) {
    return stmt->length >= 0 && xtbIsThenToken( stmt->tokens[0] );
}

bool xtbIsElseToken(Token* tk) {
    return tk != NULL && tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_ELSE;
}

bool xtbIsRemToken(Token* tk) {
    return tk != NULL && tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_REM;
}

bool xtbIsElseStatement(Statement* stmt) {
    return stmt->length >= 0 && xtbIsElseToken( stmt->tokens[0] );
}

bool xtbIsRemStatement(Statement* stmt) {
    return stmt->length >= 0 && xtbIsRemToken( stmt->tokens[0] );
}

XTBLabel* XTBaseApp::addLbl(char* name, int addr) {
    // FIXME : manage NB_LBL_MAX
    sprintf(xtb_lbls[xtb_lblCursor]->name, "%s", name);
    xtb_lbls[xtb_lblCursor]->addr = addr;

    return xtb_lbls[xtb_lblCursor++];
}

int XTBaseApp::addStatement(int type, Token** tokens, int length) {
    XTBStatement* stmt = xtb_statements[ xtb_nbStatements ];
    if ( stmt == NULL ) { 
        xtb_statements[ xtb_nbStatements ] = newStatement();
        stmt = xtb_statements[ xtb_nbStatements ]; 
    }
    
    stmt->addr = xtb_nbStatements;
    stmt->type = type;
    stmt->dest = -100; // not initialized

//printf("(ii) malloc tokens \n");

    stmt->tokens = (Token**)my_malloc( length * sizeof( Token* ) ); // FIXME
    for(int i=0; i < length; i++) {
        stmt->tokens[i] = tokens[i];
    }

    stmt->length = length;

    xtb_statements[ stmt->addr ] = stmt;

    xtb_nbStatements++;
    return stmt->addr;
}

void XTBaseApp::parseStatements() {
    cleanStatements();

    int tkAddr = 0;
    while( xtb_tokenized_prgm[tkAddr] != NULL && xtb_tokenized_prgm[tkAddr]->tkType != XTB_TK_EOP ) {
        XTBToken* tk = xtb_tokenized_prgm[tkAddr];
        XTBToken* nextTk = tkAddr+1 >= XTB_TOKENIZED_PRG_LEN ? NULL : xtb_tokenized_prgm[tkAddr+1];

        if ( xtbIsAspecificStatement( tk ) ) {
            if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_LBL ) {
                if ( nextTk != NULL && nextTk->tkType == XTB_TK_STR ) {
                    XTBToken* toks[2] = { tk, nextTk };
                    int stmtAddr = addStatement( XTB_STMT_LABEL, toks, 2 );

                    addLbl( xtbGetString( nextTk ), stmtAddr );

                    tkAddr++;
                } else {
                    error("Lbl \"<str>\" ", 1);
                }
            }
            else if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_GOSUB ) {
                if ( nextTk != NULL && nextTk->tkType == XTB_TK_STR ) {
                    XTBToken* toks[2] = { tk, nextTk };
                    addStatement( XTB_STMT_JUMP, toks, 2 );

                    tkAddr++;
                } else {
                    error("Gosub \"<str>\" ", 1);
                }
            }
            else if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_GOTO ) {
                if ( nextTk != NULL && nextTk->tkType == XTB_TK_STR ) {
                    XTBToken* toks[2] = { tk, nextTk };
                    addStatement( XTB_STMT_JUMP, toks, 2 );

                    tkAddr++;
                } else {
                    error("Goto \"<str>\" ", 1);
                }
            }
            else if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_RETURN ) {
                const int len = 1;
                XTBToken* toks[len];
                for(int i=0; i < len; i++) { toks[i] = xtb_tokenized_prgm[tkAddr+i]; }
                addStatement( XTB_STMT_JUMP, toks, len );
            }
            else if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_FOR ) {
                int runTkAddr = tkAddr;
                while( xtb_tokenized_prgm[runTkAddr] != NULL && 
                        xtb_tokenized_prgm[runTkAddr]->tkType != XTB_TK_EOP && 
                        xtb_tokenized_prgm[runTkAddr]->tkType != XTB_TK_EOL ) {
                    runTkAddr++;
                }
                const int len = runTkAddr-tkAddr;
                XTBToken* toks[len];
                for(int i=0; i < len; i++) { toks[i] = xtb_tokenized_prgm[tkAddr+i]; }
                addStatement( XTB_STMT_LOOP, toks, len );
                tkAddr = runTkAddr;
            }
            else if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_IF ) {
                int runTkAddr = tkAddr;
                while( xtb_tokenized_prgm[runTkAddr] != NULL && 
                        xtb_tokenized_prgm[runTkAddr]->tkType != XTB_TK_EOP && 
                        xtb_tokenized_prgm[runTkAddr]->tkType != XTB_TK_EOL ) {
                    runTkAddr++;
                }
                const int len = runTkAddr-tkAddr;
                XTBToken* toks[len];
                for(int i=0; i < len; i++) { toks[i] = xtb_tokenized_prgm[tkAddr+i]; }
                addStatement( XTB_STMT_CONDITIONAL, toks, len );
                tkAddr = runTkAddr;
            }
            else if ( tk->tkType == XTB_TK_KEYW && tk->iValue == XTB_KW_END ) {
                const int len = 1;
                XTBToken* toks[len];
                for(int i=0; i < len; i++) { toks[i] = xtb_tokenized_prgm[tkAddr+i]; }
                addStatement( XTB_STMT_REGULAR, toks, len );
            }
            
        }

        else if ( tk->tkType == XTB_TK_EOL ) {
            // nothing todo
        }

        else {
            int runTkAddr = tkAddr;
            while( xtb_tokenized_prgm[runTkAddr] != NULL && 
                    xtb_tokenized_prgm[runTkAddr]->tkType != XTB_TK_EOP && 
                    xtb_tokenized_prgm[runTkAddr]->tkType != XTB_TK_EOL ) {
                runTkAddr++;
            }
            const int len = runTkAddr-tkAddr;
            XTBToken* toks[len];
            for(int i=0; i < len; i++) { toks[i] = xtb_tokenized_prgm[tkAddr+i]; }
            int stmtNum = addStatement( XTB_STMT_REGULAR, toks, len );
            tkAddr = runTkAddr;
        }

        tkAddr++;
    }
}

void XTBaseApp::parseStatementsJumps() {
    for(int i=0; i < xtb_nbStatements; i++) {
        XTBStatement* stmt = xtb_statements[i];
        if (stmt->type == XTB_STMT_JUMP) {
            XTBToken* jumpInstr = stmt->tokens[0];
            if ( jumpInstr->tkType == XTB_TK_KEYW && jumpInstr->iValue == XTB_KW_RETURN ) {
                // will have to use a jumpStack
                stmt->dest = -1;
            } else if ( jumpInstr->tkType == XTB_TK_KEYW && jumpInstr->iValue == XTB_KW_GOTO ) {
                XTBLabel* lbl = xtbGetLabel(xtbGetString( stmt->tokens[1] ));
                if ( lbl == NULL ) {
                    error( "Label not found in Goto", 1 );
                    return;
                }
                stmt->dest = lbl->addr;
            } else if ( jumpInstr->tkType == XTB_TK_KEYW && jumpInstr->iValue == XTB_KW_GOSUB ) {
                XTBLabel* lbl = xtbGetLabel(xtbGetString( stmt->tokens[1] ));
                if ( lbl == NULL ) {
                    error( "Label not found in Gosub", 1 );
                    return;
                }
                stmt->dest = lbl->addr;
            } 
        }
    }
}


void XTBaseApp::parseStatementsEnds() {
    for(int i=0; i < xtb_nbStatements; i++) {
        XTBStatement* stmt = xtb_statements[i];
        if (stmt->type == XTB_STMT_REGULAR) {
            XTBToken* firstInstr = stmt->tokens[0];
            if ( firstInstr->tkType == XTB_TK_KEYW && firstInstr->iValue == XTB_KW_END ) {
                if ( i == 0 ) {
                    error("Can't have End here",1 );
                    return;
                }
                for(int j=i-1; j >= 0 ; j--) {
                    XTBStatement* upperStmt = xtb_statements[j];

                    if ( xtbIsElseStatement( upperStmt ) ) {
                        if ( upperStmt->dest < 0 ) {
                            upperStmt->dest = stmt->addr;
                            stmt->dest = upperStmt->addr;
                            int elseAddr = upperStmt->addr;

                            for(;j >= 0; j--) {
                                upperStmt = xtbGetStatement(j);
                                if ( upperStmt->type == XTB_STMT_CONDITIONAL && xtbIsThenStatement(xtbGetStatement(j+1)) ) {
                                    upperStmt->dest = elseAddr;
                                }
                            }

                            break;
                        }
                    }

                    if ( upperStmt->type == XTB_STMT_LOOP || upperStmt->type == XTB_STMT_CONDITIONAL ) {

                        if ( upperStmt->type == XTB_STMT_CONDITIONAL ) {
                            XTBStatement* thenStmt = xtb_statements[j+1];
                            if ( !xtbIsThenStatement(thenStmt) ) {  
                                // not Then -> no End to look for
                                continue;
                            }
                        }

                        if ( upperStmt->dest < 0 ) {
                            upperStmt->dest = stmt->addr;
                            stmt->dest = upperStmt->addr;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void XTBaseApp::printStatements() {
    for(int i=0; i < xtb_nbStatements; i++) {
        printStatement( xtb_statements[i] );
    }
}



bool XTBaseApp::treatStatement(XTBStatement* stmt) {
    if ( stmt->type == XTB_STMT_JUMP ) {
        return treatJumpStatement(stmt);
    } else if ( stmt->type == XTB_STMT_LABEL ) {
        // nothing
        return true;
    } else if ( stmt->type == XTB_STMT_CONDITIONAL ) {
        return treatConditionalStatement(stmt);
    } else if ( stmt->type == XTB_STMT_LOOP ) {
        return treatLoopStatement(stmt);
    } else {
        return treatRegularStatement(stmt);
    }
    error("Unknown Statement type", 1);
    return false;
}

bool XTBaseApp::treatJumpStatement(Statement* stmt) {
    Token* jumpInstr = stmt->tokens[0];
    if ( jumpInstr->iValue == XTB_KW_GOSUB ) {
        pushGosub( stmt );
        jumpTo(stmt->dest);
    } else if ( jumpInstr->iValue == XTB_KW_GOTO ) {
        jumpTo(stmt->dest);
    } else if ( jumpInstr->iValue == XTB_KW_RETURN ) {
        Jump* jp = popGosub();
        if ( jp == NULL ) {
            error("Mis Return", 1);
            return false;
        }
        jumpTo(jp->from +1);
    } 
    return true;
}

bool XTBaseApp::treatConditionalStatement(Statement* stmt) {
    int unitLen = 0;
    Token* unit = getUnit( stmt->tokens, stmt->length, 1, unitLen );

    if ( unitLen <= 0 ) {
        error("If <cond>", 1);
        return false;
    }

    bool cond = unit->fValue != 0.0;
    if ( unit->toBeDestroyed ) { my_free(unit); }

    bool hasThen = xtbIsThenStatement( xtbGetStatement( stmt->addr+1 ) );

    if ( cond ) {
        // just let PC++ -> Then -> JobToDo
    } else {
        if ( !hasThen ) {
            jumpTo( stmt->addr+2 );
        } else {
            // jumpTo( matchingEnd/Else +1 )
            jumpTo( stmt->dest+1 );
        }
    }

    return true;
}

bool XTBaseApp::treatLoopStatement(Statement* stmt) {

    Token* loopInstr = stmt->tokens[0];
    if ( loopInstr->tkType == XTB_TK_KEYW && loopInstr->iValue == XTB_KW_FOR ) {
        Variable* iterOn = xtbGetVar( stmt->tokens[1] );
        if ( iterOn == NULL ) {
            error("For <Var>,<NumExpr>,<NumExpr>[,<NumExpr>] a",1);
            return false;
        }

        //                For Var ','
        int localCounter = 0 + 1 + 1 +1;

        int unitLen = 0;
        Token* left = getUnit(stmt->tokens, stmt->length, localCounter, unitLen);
        if ( left == NULL || unitLen <= 0 ) {
            error("For <Var>,<NumExpr>,<NumExpr>[,<NumExpr>] b",2);
            return false;
        }
        float leftValue = left->fValue;
        if ( left->toBeDestroyed ) { my_free(left); }

        localCounter += unitLen + 1; // <len> + ','

        Token* right = getUnit(stmt->tokens, stmt->length, localCounter, unitLen);
        if ( right == NULL || unitLen <= 0 ) {
            error("For <Var>,<NumExpr>,<NumExpr>[,<NumExpr>] c",3);
            return false;
        }
        float rightValue = right->fValue;
        if ( right->toBeDestroyed ) { my_free(right); }

        localCounter += unitLen + 1; // <len> + ','

        Token* incrementor = getUnit(stmt->tokens, stmt->length, localCounter, unitLen);
        float incValue = incrementor == NULL || unitLen <= 0 ? 1.0 : incrementor->fValue;
        if ( incrementor != NULL && incrementor->toBeDestroyed ) { my_free(incrementor); }

        // will have to reset that value 
        bool alreadyVisited = loopInstr->fValue != 0.0;

        if ( !alreadyVisited ) {
            iterOn->fValue = leftValue;
        }

        bool minIsLeft = (leftValue <= rightValue);

        if ( minIsLeft ) {
            if ( iterOn->fValue >= rightValue ) {
                loopInstr->fValue = 0.0; // reset visitedFlag
                // jumpTo(matchingEnd +1)
                jumpTo( stmt->dest + 1 );
            }
        } else {
            if ( iterOn->fValue <= rightValue ) {
                loopInstr->fValue = 0.0; // reset visitedFlag
                // jumpTo(matchingEnd +1)
                jumpTo( stmt->dest + 1 );
            }
        }
        
        if ( alreadyVisited ) {
            iterOn->fValue += incValue; // let's increment Var
        } else {
            loopInstr->fValue = 1.0; // set visitedFlag
        }

        return true;
    }

    return false;
}

void XTBaseApp::freeArgs(Token** args, int length, bool force) {
    // if ( !force ) { return; }
    if ( force ) {
        for(int i=0; i < length; i++) {
            if ( args[i] != NULL && args[i]->toBeDestroyed ) { my_free( args[i] ); args[i] = NULL; }
        }
    }
    my_free(args, length * sizeof(Token*) );
} 

bool XTBaseApp::treatRegularStatement(Statement* stmt) {

    if ( xtbIsRemStatement( stmt ) ) {
        // Rem does nothing
        return true;
    }

    if ( xtbIsElseStatement( stmt ) ) {
        jumpTo( stmt->dest+1 );
        return true;
    }
    if ( xtbIsThenStatement( stmt ) ) {
        return true;
    }

    Token* firstInstr = stmt->tokens[0];
    if ( firstInstr->tkType == XTB_TK_KEYW && firstInstr->iValue == XTB_KW_END ) {
        Statement* dest = xtbGetStatement( stmt->dest );
        if ( dest->type == XTB_STMT_LOOP ) {
            jumpTo( dest->addr );
        }
    }

    // must also been parsed in expression(...) as a scalarValue
    if ( firstInstr->tkType == XTB_TK_FCT ) {
        if ( stmt->length < 3 ) {
            error("funct(..)",1);
            return false;
        }
        Token* nextToken = stmt->tokens[1];
        if ( nextToken->tkType != XTB_TK_PARENTOP ) {
            error("funct(..)",2);
            return false;
        }

        int length = stmt->length; // can only have <= stmt->length


        int t_size = length * sizeof( Token* );
        Token** args = (Token**)my_malloc( t_size ); // FIXME
        int argNb = 0;
        args[argNb++] = firstInstr;

        int localPointer = 1+1; // Cf '('
        for(int i=1; i < length; i++) {
            int unitLen = 0;
            args[argNb++] = getUnit( stmt->tokens, stmt->length, localPointer, unitLen );
            if ( unitLen <= 0 ) {
                break;
            }
            localPointer += unitLen;
            if ( localPointer >= stmt->length ) {
                break;
            }
            if ( stmt->tokens[localPointer]->tkType == XTB_TK_PARENTCL ) {
                break;
            }
            if ( stmt->tokens[localPointer]->tkType == XTB_TK_ARGSEP ) {
                localPointer++;
            } else {
                error("Syntax error",1);
                return false;
            }
        }

        // (!!) args[0] is the Instr itself
        // so argc is always >= 1
        (*fct_fun_ptr[firstInstr->iValue])( this, argNb, args );

        freeArgs(args, length);
        // my_free(args, t_size);
    }

    if ( firstInstr->tkType == XTB_TK_KEYW ) {
        int length = stmt->length; // can only have <= stmt->length

        int t_size = length * sizeof( Token* ); 
        Token** args = (Token**)my_malloc( t_size ); // FIXME
        int argNb = 0;
        args[argNb++] = firstInstr;

        int localPointer = 1;
        for(int i=1; i < length; i++) {
            int unitLen = 0;
            args[argNb++] = getUnit( stmt->tokens, stmt->length, localPointer, unitLen );
            if ( unitLen <= 0 ) {
                break;
            }
            localPointer += unitLen;
            if ( localPointer >= stmt->length ) {
                break;
            }
            if ( stmt->tokens[localPointer]->tkType == XTB_TK_ARGSEP ) {
                localPointer++;
            } else {
                error("Syntax error",2);
                return false;
            }
        }

        // (!!) args[0] is the Instr itself
        // so argc is always >= 1
        (*kw_fun_ptr[firstInstr->iValue])( this, argNb, args );

        freeArgs(args, length);
        // my_free(args, t_size);
    }

    if ( firstInstr->tkType == XTB_TK_VAR ) {
        Token* nextInstr = stmt->tokens[1];
        if ( nextInstr->tkType == XTB_TK_ASSIGN ) {
            Variable* var = xtbGetVar( firstInstr );
            if ( var == NULL ) {
                error("Var Assign error (0x01)", 1);
                return false;
            }

            int unitLen = 0;
            Token* value = getUnit( stmt->tokens, stmt->length, 0+1+1, unitLen );
            if ( unitLen <= 0 ) {
                error("Var Assign error (0x02)", 2);
                return false;
            }
            var->fValue = value->fValue;
            if ( value->toBeDestroyed ) { my_free(value); value = NULL; }
        } else {
            error("Var Assign error (0x03)", 3);
            return false;
        }
    }

    return true;
}


