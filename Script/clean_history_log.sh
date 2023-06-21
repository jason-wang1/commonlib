#!/usr/bin/bash

## 服务版本路径
SERVER_SEMVER_PATH=$1

## 服务执行文件
SERVER_EXE=$2

## 服务端口
SERVER_PORT=$3

## 服务路径
SERVER_PATH=${SERVER_SEMVER_PATH%/*}

## 服务日志路径
SERVER_LOG_DIR="${SERVER_PATH}/Log${SERVER_EXE}"

## 清理日志
CLEAN_LOG_PATH="${SERVER_SEMVER_PATH}/clean_log"
CLEAN_LOG_NAME="${CLEAN_LOG_PATH}/clean_msg_$(date +"%Y%m%d").log"
TLOG_INFO="[ $(date +"%Y-%m-%d %H:%M:%S") INFO ] "

## 清理日志时间
CLEAN_DATE_TIME=$(date -d"72 hour ago" +%Y%m%d_%H)

if [ ! -e "${CLEAN_LOG_PATH}" ]; then
    mkdir -p "${CLEAN_LOG_PATH}"
fi

echo "-------------------------------------------------------------" >>${CLEAN_LOG_NAME}
echo "------- start clean the log files $(date +"%Y-%m-%d %H:%M:%S") -------" >>${CLEAN_LOG_NAME}
echo "--------------- clean date ${CLEAN_DATE_TIME} before ---------------" >>${CLEAN_LOG_NAME}
echo "-------------------------------------------------------------" >>${CLEAN_LOG_NAME}

## 获取需要清理的文件
function GetNeedCleanFiles() {
    sServerLogDir=$1
    sServerPort=$2
    sCleanDateTime=$3

    ## 历史日志清理
    file_count=0
    log_count=$(find ${sServerLogDir} -name "*_${sServerPort}.*[OGRA].20*" -type f | wc -l)
    if [ ${log_count} -gt 0 ]; then
        FullFileList=$(ls -tr ${sServerLogDir}/*[OGRA].20*)

        info_link_count=$(find ${sServerLogDir} -name "*INFO" -type l | wc -l)
        data_link_count=$(find ${sServerLogDir} -name "*DATA" -type l | wc -l)
        warning_link_count=$(find ${sServerLogDir} -name "*WARNING" -type l | wc -l)
        error_link_count=$(find ${sServerLogDir} -name "*ERROR" -type l | wc -l)

        for sFullFile in ${FullFileList}; do
            sFileDateTime=${sFullFile##*.}
            if [ "${sFileDateTime}" \< "${sCleanDateTime}" ]; then
                NeedCleanFileList[$file_count]="${sFullFile}"
                ((file_count++))

                sFileName=${sFullFile##*/}

                if [ ${info_link_count} -gt 0 ]; then
                    sInfoLink=$(ls ${sServerLogDir}/*.INFO)
                    if [ -L "${sInfoLink}" ]; then
                        sInfoLinkFile=$(readlink ${sInfoLink})
                        if [ ${sInfoLinkFile} = ${sFileName} ]; then
                            NeedCleanFileList[$file_count]="${sInfoLink}"
                            ((file_count++))
                        fi
                    fi
                fi

                if [ ${data_link_count} -gt 0 ]; then
                    sDataLink=$(ls ${sServerLogDir}/*.DATA)
                    if [ -L "${sDataLink}" ]; then
                        sDataLinkFile=$(readlink ${sDataLink})
                        if [ ${sDataLinkFile} = ${sFileName} ]; then
                            NeedCleanFileList[$file_count]="${sDataLink}"
                            ((file_count++))
                        fi
                    fi
                fi

                if [ ${warning_link_count} -gt 0 ]; then
                    sWarningLink=$(ls ${sServerLogDir}/*.WARNING)
                    if [ -L "${sWarningLink}" ]; then
                        sWarningLinkFile=$(readlink ${sWarningLink})
                        if [ ${sWarningLinkFile} = ${sFileName} ]; then
                            NeedCleanFileList[$file_count]="${sWarningLink}"
                            ((file_count++))
                        fi
                    fi
                fi

                if [ ${error_link_count} -gt 0 ]; then
                    sErrorLink=$(ls ${sServerLogDir}/*.ERROR)
                    if [ -L "${sErrorLink}" ]; then
                        sErrorLinkFile=$(readlink ${sErrorLink})
                        if [ ${sErrorLinkFile} = ${sFileName} ]; then
                            NeedCleanFileList[$file_count]="${sErrorLink}"
                            ((file_count++))
                        fi
                    fi
                fi
            fi
        done
    fi

    echo ${NeedCleanFileList[*]}
}

WaitCleanFileList=($(GetNeedCleanFiles ${SERVER_LOG_DIR} ${SERVER_PORT} ${CLEAN_DATE_TIME}))

i=0
while [ $i -lt ${#WaitCleanFileList[@]} ]; do
    sFullFile=${WaitCleanFileList[i]}

    if [ -h "${sFullFile}" ];then
        rm ${sFullFile} -f
        echo "${TLOG_INFO}rm ${sFullFile} -f" >>${CLEAN_LOG_NAME}
    else
        >${sFullFile} && rm ${sFullFile} -f
        echo "${TLOG_INFO}>${sFullFile} && rm ${sFullFile} -f" >>${CLEAN_LOG_NAME}
    fi

    ((i++))

done
