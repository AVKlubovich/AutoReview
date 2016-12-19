<?php

class DBManager
{
    private static function connection()
    {
        $connect = pg_connect("host=".Config::HOST_DB.
                    " port=".Config::PORT_DB.
                    " dbname=".Config::NAME_DB.
                    " user=".Config::USER_DB. 
                    " password=".Config::PASS_DB);

        if(!$connect)
            return Errors::CALL_DB;
        return $connect;
    }
    
    public static function selectData($sqlQuery, $parameters) //errors: CONNECTION_DB
    {
        $dbconnect = DBManager::connection();
        if(is_string($dbconnect))
            return $dbconnect;
	$result = pg_prepare($dbconnect, "", $sqlQuery);
        if(!$result)
            return Errors::CALL_DB;
        $result = pg_execute($dbconnect, "", $parameters); 
        if(!$result)
            return Errors::CALL_DB;
        $response = array();
        while ($row = pg_fetch_assoc($result)) 
            array_push($response, $row);
        
        return $response;
    }
}

?>