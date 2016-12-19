<?php

class Errors
{
    const INCORRECT_RESPOSE = "-1";
    const NOT_SEND_SET_PHOTOS = "-2";
    const NOT_SEND_DATA_TO_REMORE_SERVER = "-3";
    const CALL_DB = "-4";
    const NOT_SEND_FIELD = "-5";
    const NOT_SAVE_FILE = "-6";
    const COMPANY_URL_NOT_FOUND = "-7";
    
    private $data = NULL;
    private static $current = NULL;
    
    public static function instance()
    {
        if (is_null(Errors::$current))
            Errors::$current = new Errors();
        return Errors::$current;
    }
    
    private function __construct()
    { 
        $this->data = array(
            Errors::NOT_SAVE_FILE => 'Не удалось сохранить полученный файл.',
            Errors::NOT_SEND_FIELD => 'Не отправлено одно из обязательных полей.',
            Errors::INCORRECT_RESPOSE => 'Запрос не обработан на сервере.',
            Errors::CALL_DB => 'Ошибка обращения к базе данных.',
            Errors::NOT_SEND_SET_PHOTOS => 'Не отправлен запрос "set_photos".',
            Errors::NOT_SEND_DATA_TO_REMORE_SERVER => 'Не удалось отправить данные на удаленный сервер.',
            Errors::COMPANY_URL_NOT_FOUND => 'Не найден url компании.'
        );
    }
    
    public function data($key)
    {
        return $this->data[$key];
    }
}

?>
