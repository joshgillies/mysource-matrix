<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: weatherco_feed.inc,v 1.1 2007/10/24 05:46:29 bshkara Exp $
*
*/

check_ssv_licence();



require_once SQ_PACKAGES_PATH.'/data/data_source_xml/data_source_xml.inc';

/**
* Weather Company Feed
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Weatherco_Feed extends Data_Source_Xml
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function Weatherco_Feed($assetid=0)
    {
        $this->Data_Source_Xml($assetid);

	}//end constructor


	/**
	* Used for parsing the XML file
	*
	* @return void
	* @access public
	*/
	function parseXML()
	{
		$url = html_entity_decode($this->attr('url'));
		$userid = $this->attr('userid');
		$password = $this->attr('password');
		// calculate Weather Co. rolling authentication key
		$day = date('j');
		$month = date('n');
		$year = str_replace('0', '', date('y'));
		$authkey = md5((($day * 2) + ($month * 300) + ($year * 170000)).$password);

        $xml_parser = xml_parser_create();
        // stop the parser upper-casing everything
		xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);
		xml_set_character_data_handler ($xml_parser, Array(&$this,'characterData') );
        xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));

		// initialise file handler
		if (!($fp = fopen($url.'&u='.$userid.'&k='.$authkey, 'r'))) {
			trigger_localised_error('CORE0257', E_USER_WARNING, $import_path);
			return FALSE;
			die();
		}

		// read the XML file 4KB at a time
		$block_size = 4096;
		while ($data = fread($fp, $block_size)) {
            if (!xml_parse($xml_parser, $data, feof($fp))) {
                trigger_localised_error('CORE0258', E_USER_WARNING, xml_error_string(xml_get_error_code($xml_parser)), htmlentities($data));
                return FALSE;
            }
		}

		// clean up
		fclose($fp);
		xml_parser_free($xml_parser);

		return $this->_result;

	}//end parseXML()


}//end class
?>
