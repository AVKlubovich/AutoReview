<?php

class AddDamage extends BaseFile
{
    public function exec(&$assoc)
    {
        $url_files = "http://$_SERVER[SERVER_ADDR]/".Config::TARGET_DIR_FILES;

        $is_send_file = false;
        if (!empty($_FILES))
            $is_send_file = true;

        if (!$is_send_file ||
            !array_key_exists('id_car', $assoc))
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD),
                    'field' => "id_car",
                    'is_send_file' => $is_send_file));
        }

        $idCar = $assoc["id_car"];
        if (count($idCar) <= 0)
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD),
                    'field' => "id_car"));
        }
        
        //echo'<pre>';print_r($_FILES); echo'</pre>';
        //echo'<pre>';print_r($assoc); echo'</pre>';die;

        foreach ($assoc["damages"] as $key => &$valueDamage)
        {
            $idElement = $valueDamage["id_element"];
            $idDamage = $valueDamage["id_damage"];
            $photos = &$valueDamage["photos"];
            
            echo'<pre>';print_r($photos); echo'</pre>';

            $urls = array();
            foreach ($photos as $photoName)
            {
                $fileIsFinded = false;
                foreach ($_FILES as $fileArrName => $file)
                {
                    $fileName = $file["name"];
                    
                    if ($fileName == $photoName)
                    {
                        $savedFileName = $this->saveFile(Config::TARGET_DIR_FILES, $file);
                        $fileUrl = "$url_files$savedFileName";
                        array_push($urls, $fileUrl);
                        
                        $fileIsFinded = true;
                        continue;
                    }
                }
                if ($fileIsFinded == false)
                {
                    Utils::printData(
                        array('status' => Errors::NOT_SEND_SET_PHOTOS,
                              'error' => Errors::instance()->data(Errors::NOT_SEND_SET_PHOTOS)
                              )
                    );
                }
            }

//            $valueDamage["photos"] = $urls;
            $valueDamage["urls"] = $urls;
            
        }

//        echo'<pre>';print_r($assoc); echo'</pre>';die;

        $assoc["status"] = 1;
        $assoc["type_command"] = "save_damage_car";

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