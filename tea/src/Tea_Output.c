/*
 *
 *   TEA - A quick and simple text preprocessor
 *   Copyright (C) 2010-2016 DarkBatcher
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <libpBat.h>
#include <string.h>
#include "Tea.h"

size_t         Tea_GetWordLengthT(const char* lpBegin, const TEANODE* lpTeaNode)
{
	size_t iLength=0;

	while (lpBegin && *lpBegin!=' ' && *lpBegin!='\0'
	       && *lpBegin!='\n') {


		if (*lpBegin=='\0') {

			/* on passe au noeud suivant */

			do {

				lpTeaNode=lpTeaNode->lpTeaNodeNext;

				if (lpTeaNode)
					lpBegin=lpTeaNode->lpContent;

			} while (lpTeaNode && *lpBegin=='\0');

			if (*lpBegin=='\0')
				goto Tea_GetWordLength_End;


		} else {

			lpBegin=pBat_GetNextChar(lpBegin);

		}

		iLength++;


	}

Tea_GetWordLength_End:

	return iLength;

}

size_t      Tea_GetWordLength(char* lpBegin)
{
	size_t iLength=0;

	while (*lpBegin
	       && *lpBegin!=' '
	       && *lpBegin!='\n') {

		lpBegin=pBat_GetNextChar(lpBegin);
		iLength++;

	}

	return iLength;
}

void        Tea_MakeMargin(size_t iLength, size_t* iLeft, FILE* pFile)
{
	while (iLength) {

		putc(' ', pFile);

		iLength--;
		(*iLeft)--;

	}

}

char*       Tea_OutputWord(char* lpBegin, FILE* pFile, size_t* iLeft)
{
	char* lpNext;

	while (*lpBegin
	       && *lpBegin!=' '
	       && *lpBegin!='\n'
	       && *iLeft) {


		lpNext=pBat_GetNextChar(lpBegin);

		while (lpBegin < lpNext) {

			/* This is somewhat important since it takes
			   account of possibly mutiple byte sequences */

			fputc(*lpBegin,pFile);
			lpBegin++;

		}

		(*iLeft)--;

	}

	/* return that there's no more pending characters */
	if (!*lpBegin)
		return NULL;

	return lpBegin;
}

char*       Tea_OutputLineT(char* lpBegin, FILE* pFile, TEANODE* lpTeaNode, size_t* iLeft)
{
	const char* const lpStart = lpBegin;
	const char* const lpEnd = lpBegin ? strchr(lpBegin, '\0') : NULL;
	while (lpBegin && *lpBegin && *iLeft > 0) {
		/* if we are at the start of the line, skip any whitespace. */
		if (lpBegin == lpStart) {
			while (*lpBegin == ' '
				&& fseek(pFile, -1, SEEK_CUR) == 0
				&& getc(pFile) == '\n') {
				lpBegin++;
				(*iLeft)--;
			}
		}

		/* find the start of the word */
		const char* lpWordStart = lpBegin;
		while (*lpWordStart == ' ') {
			lpWordStart++;
		}

		const size_t iWordLen = Tea_GetWordLengthT(lpWordStart, lpTeaNode);
		if (iWordLen + (lpWordStart - lpBegin) < *iLeft) {
			const char* lpNextWordStart = lpWordStart + iWordLen;
			while (lpNextWordStart < lpEnd && *lpNextWordStart == ' ') {
				lpNextWordStart++;
			}
			const size_t iNextWordLen = lpWordStart >= lpNextWordStart && lpTeaNode->lpTeaNodeNext
				? Tea_GetWordLengthT(lpTeaNode->lpTeaNodeNext->lpContent, lpTeaNode->lpTeaNodeNext)
				: 2 + (lpNextWordStart - lpWordStart);
			while (*lpBegin == ' ' && *lpWordStart != '\n' && (lpWordStart < lpNextWordStart || *iLeft > iNextWordLen)) {
				fputc(*lpBegin, pFile);
				lpBegin++;
				(*iLeft)--;
			}

			lpBegin = iWordLen > 0
				? Tea_OutputWord(lpBegin, pFile, iLeft)
				: NULL;

			/* if the line is finished, return */
			if (lpBegin && *lpBegin == '\n') {
				lpBegin++;
				(*iLeft)--;
				break;
			}
		}
		else {
			break;
		}
	}

	return lpBegin;
}

/**
 * @brief Outputs a line of text to a file.
 *
 * This function writes a line of text starting from the given pointer `lpBegin` to the specified file `pFile`.
 * The number of characters left to write is updated in the `iLeft` parameter.
 *
 * @param lpBegin Pointer to the beginning of the line of text to be written.
 * @param pFile Pointer to the file where the line of text will be written.
 * @param iLeft Pointer to a size_t variable that holds the number of characters left to write. This value is updated by the function.
 *
 * @return Pointer to the next character after the written line in the input string.
 */
char*       Tea_OutputLine(char* lpBegin, FILE* pFile, size_t* iLeft)
{
	size_t iNextWordLen;

	while (*lpBegin) {

		iNextWordLen=Tea_GetWordLength(lpBegin);

		/* the line is obviously far too big
		   let the user do what they want with new lines
		*/

		if (iNextWordLen >= *iLeft) return lpBegin;

		lpBegin=Tea_OutputWord(lpBegin, pFile, iLeft);

		/* if there is no pending character */
		if (!lpBegin)
			return NULL;

		/* if the line is finished, return */
		if (*lpBegin=='\n')
			return lpBegin;

		/* #jve #todo Review correctness */
		/* if there are more words coming, add the next character and move on */
		if (*lpBegin != '\0' && *lpBegin != '\n' && 1 <= *iLeft) {
			fputc(*lpBegin, pFile);
			(*iLeft)--;
		}

		/* any way, the words are separated by spaces */
		lpBegin++;

	}

	return NULL;
}
