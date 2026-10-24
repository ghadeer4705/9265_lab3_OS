#include "tokenizer.h"
#include <sstream>
#include <iostream>
#include <cctype>


static bool readWord(const std::string &input, size_t &i, std::string &word, bool &quoted)
{
    word.clear();
    quoted = false;

    while (i < input.size() && isspace((unsigned char)input[i]))
        i++;

    if (i >= input.size())
        return false;

    if (input[i] == '|')
    {
        word = "|";
        i++;
        return true;
    }

    if (input[i] == '<')
    {
        word = "<";
        i++;
        return true;
    }

    if (input[i] == '>')
    {
        word = ">";
        i++;

        if (i < input.size() && input[i] == '>')
        {
            word += ">";
            i++;
        }

        if (i < input.size() && input[i] == '&')
        {
            word += "&";
            i++;
        }

        return true;
    }

    if (input[i] == '2' && i + 1 < input.size() && input[i + 1] == '>')
    {
        word = "2>";
        i += 2;
        return true;
    }

    while (i < input.size() && !isspace((unsigned char)input[i]) &&
           input[i] != '|' && input[i] != '<' && input[i] != '>')
    {
        if (input[i] == '"' || input[i] == '\'')
        {
            char quote = input[i++];
            quoted = true;

            while (i < input.size() && input[i] != quote)
                word += input[i++];

            if (i < input.size())
                i++;
        }
        else
        {
            word += input[i++];
        }
    }

    return true;
}

std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    std::string word;
    bool quoted;
    size_t pos = 0;
    while (readWord(input, pos, word, quoted))
    {
        Token token;

        if (word == "|")
            token.type = TOKEN_PIPE;
        else if (word == ">")
            token.type = TOKEN_REDIRECT;
        else if (word == ">>")
            token.type = TOKEN_APPEND;
        else if (word == "<")
            token.type = TOKEN_INPUT;
        else if (word == "&")
            token.type = TOKEN_BACKGROUND;
        else if (word == "2>")
            token.type = TOKEN_ERROR;
        else if (word == ">>&")
            token.type = TOKEN_REDIRECT_AND_ERROR;
        else
            {
                if (!tokens.empty())
                {
                    Token &prev = tokens.back();
                    if (prev.type == TOKEN_REDIRECT ||
                        prev.type == TOKEN_APPEND ||
                        prev.type == TOKEN_INPUT ||
                        prev.type == TOKEN_ERROR ||
                        prev.type == TOKEN_REDIRECT_AND_ERROR ||
                        prev.type == TOKEN_COMMAND ||
                        prev.type == TOKEN_ARGUMENT ||
                        word[0] == '-' ||
                        word.find('/') != std::string::npos ||
                        word.find('.') != std::string::npos

                    )
                    {
                        token.type = TOKEN_ARGUMENT;
                    }
                    else
                    {
                        token.type = TOKEN_COMMAND;
                    }
                }
                else
                {
                    token.type = TOKEN_COMMAND;
                }
            }

        token.value = word;
        tokens.push_back(token);
    }

    tokens.push_back({TOKEN_EOF, ""});
    return tokens;
}
