#! /bin/bash

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

#############################################################################
#
# This script generates a directory tree html representation of the
# Corinthia project for the local file directory and the remote Github
# directory (master).
#
# It pulls down latest Corinthia version, makes the new tree, diffs it
# to the old tree and if the tree has changed, creates, commit and
# pushes the new trees.  It uses the commit message to label the
# file's purpose.
#
# File created/updated:  
#
#    $TREE_FILE_NAME     --- the tree that works for the local directory
#    $GIT_TREE_FILE_NAME --- the tree that works for the remote repository
#
# This script is designed to run as a daily cronjob on one of the
# comitter's machines and it is assumed that their git setup has
# access to their server password.
#
# useage:  ./checktree -w
#
# Current 'tree guardian':  Gabriela <gbg@apache.org>
#
##########################################################################

PROJECT_NAME="Corinthia"
TRUNK_PATH="/home/g/cor-tree-cronjob/incubator-corinthia/"
TREE_FILE_NAME="tree_local_directory.html"
GIT_TREE_FILE_NAME="tree_github_repository.html"
PROJECT_SERVER="apache.org"
LOG_FILE="CorinthiaTree.log"
MAX_RETRIES=20
COUNT_TRIES=0

GIT_HUB_LINKS='s/href="./href=\"https:\/github.com\/apache\/incubator-corinthia\/tree\/master/g'

WRITE=0

write_log()
{
    echo "$(date):  $*." >> $LOG_FILE
}

usage() 
{
    echo "Only \"-w\" is a valid argument.  Exiting"
    exit
}

update_tree()
{
    HAS_INET_CONNECTION=$(ping -c 1 $PROJECT_SERVER | grep -c "1 received")

    if [[ $HAS_INET_CONNECTION != 1 ]]
    then
        COUNT_TRIES=$(( COUNT_TRIES+1 ))
        if [[  $MAX_RETRIES -gt $COUNT_TRIES ]]
        then     
            $(sleep "3600s")
            update_tree
        else
            write_log "Update failure"
        fi    
    else
        WANT_UPDATE=$(git pull)
        if [ "$WANT_UPDATE" == "Already up-to-date." ];
        then
            write_log "$WANT_UPDATE"
            exit
        else
            tree -d > new_tree.html
            NEED_TREE=$(diff new_tree.html old_tree.html)
            if [[ -n $NEED_TREE ]] 
            then
                mv new_tree.html old_tree.html
                tree -d -H . -T "$PROJECT_NAME Local File Directory Structure" > $TREE_FILE_NAME
                tree -d -H . -T "$PROJECT_NAME Github Current Repository Directory Structure" |
                  sed -e $GIT_HUB_LINKS > $GIT_TREE_FILE_NAME
                if [[ $WRITE == 1 ]]; 
                then
                    git add $TREE_FILE_NAME
                    git commit -m "Local File Directory Structure"
                    git push
                    git add $GIT_TREE_FILE_NAME
                    git commit -m "Github Current Repository Directory Structure"
                    git push
                    write_log "Tree updated."
                else
                    write_log "Tree not written, see useage."
                fi
            fi
        fi
    fi
}

cd $TRUNK_PATH

write_log "check_tree.sh: Activated"

while getopts w opt; do
    case "$opt" in
        w)
            WRITE=1
            ;;
        *)
            usage
    esac
done

update_tree

exit
