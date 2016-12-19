<?php

class NetworkManager
{
    public static function sendHttp($url, $assoc) //errors: NOT_SEND_DATA_REMORE_SERVER, INCORRECT_RESPOSE
    {
        $data = http_build_query($assoc);
        $options = array(
            'http' => array(
                'header'  => "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ".strlen($data),
                'method'  => 'POST',
                'content' => $data,
                )
        );
        
        $result = NetworkManager::sendResponce($url, $options);
        if ($result == Errors::NOT_SEND_DATA_TO_REMORE_SERVER)
            return $result;
        
        $obj = json_decode($result);
        if ($obj->status == Errors::INCORRECT_RESPOSE)
            return Errors::INCORRECT_RESPOSE;
        return $obj;
    }
    
    public static function downloadFile($url, $assoc) //errors: NOT_SEND_DATA_REMORE_SERVER, INCORRECT_RESPOSE
    {
        $data = http_build_query($assoc);
        $options = array(
            'http' => array(
                'header'  => "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: ".strlen($data),
                'method'  => 'POST',
                'content' => $data,
                )
        );

        $result = NetworkManager::sendResponce($url, $options);
        
        return $result;
    }
    
    public static function sendJson($url, $assoc) //errors: NOT_SEND_DATA_REMORE_SERVER, NOT_SEND_SET_PHOTOS
    {
        $json = Utils::toJson($assoc);
//        echo "<p>$json</p>";
        $options = array(
            'http' => array(
                'header'  => "Content-Type: application/json\r\nContent-Length: ".strlen($json),
                'method'  => 'POST',
                'content' => $json,
                )
        );
        
        return NetworkManager::sendResponce($url, $options);
    }
    
    static function sendResponce($url, &$options)
    {
        $context  = stream_context_create($options);
//        Utils::printData("<p>!>$context</p>");
        $result = file_get_contents($url, false, $context);

        if (empty($result))
            return Errors::NOT_SEND_DATA_TO_REMORE_SERVER;
        else
            return $result;
    }
}

?>