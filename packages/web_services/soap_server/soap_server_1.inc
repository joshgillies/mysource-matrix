<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_server_1.inc,v 1.1 2008/05/09 04:54:02 hnguyen Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Soap Server
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Server extends Page
{

	const SOAP_XML_SCHEMA_VERSION = 'http://www.w3.org/2001/XMLSchema';
	const SOAP_XML_SCHEMA_INSTANCE = 'http://www.w3.org/2001/XMLSchema-instance';
	const SOAP_SCHEMA_ENCODING = 'http://schemas.xmlsoap.org/soap/encoding/';
	const SOAP_ENVELOP = 'http://schemas.xmlsoap.org/soap/envelope/';
	const SCHEMA_SOAP_HTTP = 'http://schemas.xmlsoap.org/soap/http';
	const SCHEMA_SOAP = 'http://schemas.xmlsoap.org/wsdl/soap/';
	const SCHEMA_WSDL = 'http://schemas.xmlsoap.org/wsdl/';

	private $function_list = Array();
	private $function_info;


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
		$this->_ser_attrs = TRUE;
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns an array of all the permitted links type, the type asset and the cardinality
    *
    * @return array
    * @access private
    * @see Asset::_getAllowLinks()
    */
    public function _getAllowedLinks()
    {
        return Array(
                SQ_LINK_TYPE_2 => Array('soap_api' => Array('card' => 'M', 'exclusive' => FALSE)),
                SQ_LINK_TYPE_1 => Array('soap_api' => Array('card' => 'M', 'exclusive' => FALSE)),
                );

    }//end _getAllowedLinks()


	/**
	*
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == 'POST') {
			$this->service($HTTP_RAW_POST_DATA);
		} else {
			if (isset($_SERVER['QUERY_STRING']) && strcasecmp($_SERVER['QUERY_STRING'], 'wsdl') == 0) {
				$this->getWSDL();	
			} else {
				parent::printFrontend();
			}//end else
		}//end else

	}//end if


	/**
	*
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		if (!isset($_GET['desc'])) {
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
			foreach ($children as $api_id => $type_info) {
				$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
				$type_code = $type_info[0]['type_code'];

				$functions = unserialize($child->attr('function_list'));
				echo '<ul>';
				foreach ($functions as $func_name => $switch) {
					//if ($switch && method_exists($child, $func_name)) {
					if ($switch) {
						echo '<li style="padding-bottom:10px;"><a href="'.$_SERVER['PHP_SELF'].'?desc='.$func_name.'&group='.str_replace('soap_api_', '', $type_code).'" style="color:#336699;font-weight:bold;font-size:13px;font-family:Tahoma;">
								'.ucwords($func_name).'
							</a></li><br />';

					}//end if
				}//end if
				echo '</ul>';
			}//end foreach
		} else {
			$func_name = $_GET['desc'];
			if (isset($_GET['group'])) {
				$group = $_GET['group'];
			}//end if

			if (!empty($func_name) && !empty($group)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_api_'.$group);
				$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', 'soap_api_'.$group))));
				$method = $class->getMethod($func_name);
				$comment = $this->cleanComment($method->getDocComment());
				bam($comment);
			}//end if
		}//end else
	}//end printBody()


	function cleanComment($comment)
	{
		$comment = str_replace('/*', '' , $comment);
		$comment = str_replace('*/', '' , $comment);
		$comment = str_replace('*', '' , $comment);

		return $comment;
	}//end if


	/**
	* Description: This function call SOAP Server extension to handle requests.
	*
	* @return void
	* @access public
	*/
	function service($http_raw_post_data)
	{
		$url = $this->getUrl();
		$server = new SoapServer(NULL, Array ('uri'	=> $this->getUrl()));
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
		$function_list = Array();
		foreach ($children as $api_id => $type_info) {
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$functions = unserialize($child->attr('function_list'));
			$function_list = array_merge($function_list, array_fill_keys(array_keys($functions), $type_code));
		}//end foreach
		

	}//end service()


	/**
	* Description: This function returns the WSDL for this soap server, including all available methods of
	* all APIs linked underneath.
	*
	* @return void
	* @access public
	*/
	private function getWSDL()
	{
		//header("Content-type: text/xml");
		//$server = new SoapServer($url.'?wsdl');
		$server = new SoapServer(NULL, Array ('uri'	=> $this->getUrl()));
		//echo $this->getXML();
		$this->findFuncInfo();
		/*
		foreach ($this->function_list as $func_name => $class_name) {	
				$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', $class_name))));
				$method = $class->getMethod($func_name);
				$params = $method->getParameters();
				bam($params);
		}//end foreach
		*/
		/*$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
		$function_list = Array();
		foreach ($children as $api_id => $type_info) {
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			//$server->setClass($type_code);
		}//end foreach

		foreach ($children as $api_id => $type_info) {
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$functions = unserialize($child->attr('function_list'));
			//$function_list = array_merge($function_list, array_keys($functions));
			$function_list = array_merge($function_list, array_fill_keys(array_keys($functions), $type_code));
		}//end foreach
		bam($server->getFunctions());
		bam($function_list);*/

	}//end getWSDL()


	/**
	*
	*
	* @return void
	* @access public
	*/
	private function getXML() 
	{
		$wsdl = new DomDocument("1.0", 'utf-8');
		$root = $wsdl->createElementNS('http://schemas.xmlsoap.org/wsdl/', 'definitions');
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:xsd','http://www.w3.org/2001/XMLSchema');
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:tns', $this->ns);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:soap-env',self::SCHEMA_SOAP);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:wsdl',self::SCHEMA_WSDL);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:soapenc',self::SOAP_SCHEMA_ENCODING);
		$root->setAttribute('targetNamespace', $this->ns);

		return $wsdl->saveXML();

	}//end getXML()


	/**
	*
	*
	* @return void
	* @access public
	*/
	private function emptys()
	{

	}//end empty()


	function findFuncInfo()
	{
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
		foreach ($children as $api_id => $type_info) {
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$functions = unserialize($child->attr('function_list'));

			foreach ($functions as $key => $value) {
				$functions[$key]	= Array (
										'class_name' => $type_code,
									  );
			}//end foreach

			$this->function_list = array_merge($this->function_list, $functions);
		}//end foreach

		foreach ($this->function_list as $func_name => $func_info) {	
				$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', $func_info['class_name']))));
				if (method_exists($func_info['class_name'], $func_name))
				{
					bam($func_name);
					$method = $class->getMethod($func_name);
					$doc_comment = $method->getDocComment();
					bam($doc_comment);
					$params = $method->getParameters();
					bam($params);
					foreach ($params as $param) {
						bam(var_export($param->__toString(),TRUE));
						//bam(var_export($param->getClass(),TRUE));
						bam(var_export($param->getDeclaringClass(),TRUE));
						//we need to get the return value of the function without using docComment, PIECES OF SHIT!!
					}//end foreach
				}//end if
		}//end foreach
		
		
	}//end findFuncInfo()


}//end class
?>
