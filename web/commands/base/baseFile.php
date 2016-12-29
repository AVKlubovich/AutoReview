<?php

class BaseFile extends BaseCommand
{
    protected function getNameFile()
    {
        $date = date_create();
        return date_format($date, 'Y-m-d_H-i-s-').substr((string)microtime(), 2, 7);
    }

    protected function saveFile($folder, &$fileInfo)
    {
        $fileName = $this->getNameFile().substr($fileInfo["name"], strrpos($fileInfo["name"], ".", -1));
        if (move_uploaded_file($fileInfo["tmp_name"], $_SERVER["DOCUMENT_ROOT"]."/$folder$fileName"))
            return $fileName;
        else
        {
            Utils::printData(
                array('status' => Errors::NOT_SAVE_FILE,
                    'error' => Errors::instance()->data(Errors::NOT_SAVE_FILE)));
        }
    }

    public function exec(&$assoc)
    {
    }
}

?>