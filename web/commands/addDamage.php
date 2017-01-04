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

        foreach ($assoc["damages"] as $key => &$valueDamage)
        {
            $idElement = $valueDamage["id_element"];
            $idDamage = $valueDamage["id_damage"];
            $photos = &$valueDamage["photos"];

            $urls = array();
            foreach ($photos as $photoName)
            {
                foreach ($_FILES as $fileName => $file)
                {
                    if ($fileName == $photoName)
                    {
                        echo'<pre>';print_r($fileName); echo'</pre>';
                        echo'<pre>';print_r($file); echo'</pre>';

                        $savedFileName = $this->saveFile(Config::TARGET_DIR_FILES, $file);
                        $fileUrl = "$url_files$savedFileName";
                        array_push($urls, $fileUrl);
//                        echo'<pre>';print_r($fileUrl); echo'</pre>';
                    }
//                    echo'<pre>';print_r($file); echo'</pre>';die;
                }
//                echo'<pre>';print_r($photoName); echo'</pre>';die;
            }

            $valueDamage["photos"] = $urls;
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