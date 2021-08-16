int strlen_r(char *str){
    /**
     * Reentrant string length
     * Solution from StackOverflow
     * :param str: String
     * :return: String length
     */
    return *str == 0 ? 0 : 1+strlen_r(str+1);
}

void removeNewline(char **str){
    /**
     * Remove string newline
     * Solution from StackOverflow
     * :param str: String pointer
     * :return: Null
     */
    char *newstr = strchr(*str, '\n');
    if(newstr != NULL){
        *newstr = '\0';
    }
}
