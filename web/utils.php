<?php

class Utils
{
    public static function printData($data)
    {
        print(Utils::toJson($data));
        exit();
    }
    
    public static function printError($error)
    {
        Utils::printData(array('status' => -1, 'error' => $error));
    }
    
    public static function printOk()
    {
        Utils::printData(array('status' => 1));
    }
    
    public static function toJson($data)
    {
        return json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES);
    }
}

?>