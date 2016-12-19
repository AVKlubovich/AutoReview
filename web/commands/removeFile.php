<?php

class RemoveFile extends BaseFile
{
    public function exec(&$assoc)
    {
        if (!array_key_exists('file_url', $assoc) ||
            !array_key_exists('id_file', $assoc))
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD)));
        }
        $fileInfo = $assoc["file_url"];
        
        if (!is_array($assoc) ||
            count($fileInfo) <= 0)
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD)));
        }

        if ($this->removeFile(Config::TARGET_DIR_FILES, $fileInfo))
            $assoc["status"] = 1;
        else
            $assoc["status"] = -1;

        $result = NetworkManager::sendJson(Config::HOST_CPP, $assoc);
        if ($result == Errors::NOT_SEND_DATA_TO_REMORE_SERVER)
        {
            Utils::printData(
                array('status' => $result,
                    'error' => Errors::instance()->data($result)));
        }

        echo $result;
        exit();
    }
}

?>