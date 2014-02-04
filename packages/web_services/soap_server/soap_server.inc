<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_server.inc,v 1.19 2013/03/21 08:01:05 ewang Exp $
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
* @author  Basil Shkara <bshkara@squiz.net>
* @author  Benjamin Pearson <bpearson@squiz.net>
*
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Server extends Page
{

	// Standard Naming for MESSAGE: [function_name]SoapIn(Out)

	const SOAP_XML_SCHEMA_VERSION = 'http://www.w3.org/2001/XMLSchema';
	const SOAP_XML_SCHEMA_INSTANCE = 'http://www.w3.org/2001/XMLSchema-instance';
	const SOAP_SCHEMA_ENCODING = 'http://schemas.xmlsoap.org/soap/encoding/';
	const SOAP_ENVELOP = 'http://schemas.xmlsoap.org/soap/envelope/';
	const SCHEMA_SOAP_HTTP = 'http://schemas.xmlsoap.org/soap/http';
	const SCHEMA_SOAP = 'http://schemas.xmlsoap.org/wsdl/soap/';
	const SCHEMA_WSDL = 'http://schemas.xmlsoap.org/wsdl/';
	const XML_CONTENT_TYPE = 'Content-Type: text/xml; charset=utf-8';

	private $function_list				= Array();
	private $complex_types				= Array();
	private $complex_elements			= Array();
	private $simple_types				= Array ('float', 'int', 'string', 'boolean');
	private	$simple_restricted_types	= Array();

	private $message_node;
	private $portType_node;
	private $binding_node;
	private $schema_node;

	private $wsdl;
	private $root;

	private $server_location;
	private $server_url;


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
	* Print the frontend of the asset without the design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// Check the REQUEST METHOD
		if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == 'POST') {
			// Set raw post data if it has not been set
			if (!isset($HTTP_RAW_POST_DATA)) {
				$HTTP_RAW_POST_DATA = file_get_contents("php://input");
			}//end if
			// If we dont have read access, try to authenticate via HTTP.

			$logged_in = $this->authenticateHttpUser();
			if (!$this->readAccess()) {
				header('WWW-Authenticate: Basic realm="Squiz Matrix"');
				header('HTTP/1.0 401 Unauthorized');
				return;

			}//end if

			// Lets service our gods.
			$this->service($HTTP_RAW_POST_DATA);
		} else {
			$logged_in = $this->authenticateHttpUser();
			if (!$this->readAccess()) {
				// Just return nothing so the SoapClient will be like, wth?
			    	header('WWW-Authenticate: Basic realm="Squiz Matrix"');
				header('HTTP/1.0 401 Unauthorized');
				return;
			}//end if

			// If the query string has WSDL on it, somebody want the WSDL
			if (isset($_SERVER['QUERY_STRING']) && strcasecmp($_SERVER['QUERY_STRING'], 'wsdl') == 0) {
				$this->getWSDL();
			} else {
				// Or we just print the frontend interface to get functions/services details.
				parent::printFrontend();
			}//end else
		}//end else


	}//end if


	/**
	* This function authenticate the user via HTTP
	*
	* @return boolean
	* @access private
	*/
	private function authenticateHttpUser()
	{
		$logged_in = FALSE;
		if (!(array_get_index($_SERVER, 'PHP_AUTH_USER') && array_get_index($_SERVER, 'PHP_AUTH_PW'))) {
            return $logged_in;
        }//end if

		if (!is_null($_SERVER['PHP_AUTH_USER'])) {
			$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
			$auth_systems = $auth_folder->getAuthSystems();

	        $user = NULL;
	        foreach ($auth_systems as $systemid) {
	            $system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
	            if (is_null($system)) continue;
	            $user = $system->authenticateUser($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
	            if (!is_null($user)) {
	            	$GLOBALS['SQ_SYSTEM']->loginUser($user);
	            	$logged_in = TRUE;
	            	break;
	            }//end if
	        }//end foreach
		}//end if

        unset($_SERVER['PHP_AUTH_USER']);
        unset($_SERVER['PHP_AUTH_PW']);

		return $logged_in;

	}//end authenticateHttpUser()


	/**
	* Description: This function call SOAP Server extension to handle requests.
	*
	* @return void
	* @access public
	*/
	function service($HTTP_RAW_POST_DATA)
	{
		ini_set("soap.wsdl_cache_enabled", "0");
		require_once dirname(__FILE__).'/soap_server_error_handler.inc';

		// Set error handler
		$old_error_handler = set_error_handler('soap_server_error_handler');

		$server = new SoapServer($this->data_path.'/content_file.wsdl');

		// traverse SOAP request
		try {
			$dom = new DOMDocument();
			$dom->loadXML($HTTP_RAW_POST_DATA);
		} catch (Exception $e) {
			throw new Exception('Unable to parse SOAP request "'.$HTTP_RAW_POST_DATA.'": '.$e->getMessage());
		}//end try catch

		// pointer to the children of root
		$children = $dom->documentElement->childNodes;
		$i = 0;
		$function_names = Array();
		while ($children->item($i)) {
			// libxml element numeric node type
			if (($children->item($i)->nodeType === 1) && (strpos($children->item($i)->tagName, 'Body') !== FALSE)) {
				foreach ($children->item($i)->childNodes as $child) {
					if ($child->nodeType === 1) {
						$function_names[] = preg_replace('/(.*:)/', '', $child->tagName);
					}//end if
				}//end foreach
				break;
			}//end if
			$i++;
		}//end while

		$operations = $this->getAvailableOperations();
		for ($i=0; $i < count($function_names); $i++) {
			if (isset($operations[$function_names[$i]])) {
				$class_name = $operations[$function_names[$i]]['class_name'];
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($class_name);
				$server->setClass($class_name);
				$server->handle();
			}//end if
		}//end foreach

		// Restore error handler
		restore_error_handler();
		exit(0);

	}//end service()


	/**
	* Print the Body of the Soap Server, in this case is the interface displaying all the functions that it is offering
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$available_operations = $this->getAvailableOperations(TRUE);
		$list_api = $this->attr('list_api');
		if (!isset($_GET['desc'])) {
				echo '<ul style="list-style: circle;">';
				foreach ($available_operations as $func_name => $class_info) {
					?>
					<li style="padding-bottom:10px;">
							<a href="<?php echo $_SERVER['PHP_SELF'].'?desc='.$func_name.'&group='.str_replace('soap_api_', '', $class_info['class_name']) ?>" style="color:#336699;font-weight:bold;font-size:13px;font-family:Tahoma;">
							<?php echo ucwords($func_name) ?>
							</a>
					</li>
					<br />
					<?php
				}//end if
				echo '</ul>';
		} else {
			$func_name = $_GET['desc'];
			if (isset($_GET['group'])) {
				$group = $_GET['group'];
			}//end if

			if (!empty($func_name) && !empty($group)) {

				// PAINT DETAILS IF We only print this if the method exists/available.
				if (isset($available_operations[$func_name]) && $available_operations[$func_name]['class_name'] == 'soap_api_'.$group) {
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_api_'.$group);
					$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', 'soap_api_'.$group))));
					$method = $class->getMethod($func_name);
					?>
					<script language="Javascript" type="text/javascript">
						function revealEntry(baseloc,entryname) {
							imagename = "expand"+entryname;
							if (document.getElementById(entryname).style.display == "none") {
								document.getElementById(entryname).style.display = "";
								document[imagename].src = baseloc+"/web/images/icons/internal_message/del.png";
							}
							else {
								document.getElementById(entryname).style.display = "none";
								document[imagename].src = baseloc+"/web/images/icons/internal_message/add.png";
							}

						}
					</script>

				<span style="font-size:16px;font-family:tahoma;font-weight:bold;">
					<?php echo (($method->isPublic()) ? 'Public ' : '') ?>
				</span>
				<span style="color:#006699;font-size:16px;font-family:tahoma;font-weight:bold;">
					<?php echo ucwords($func_name) ?>
				</span>
				<br />
				<?php
					$comment = $this->cleanComment($method->getDocComment());
					$func_info = $this->getFuncDocInfo($comment);

					// EVAL FUNCTION HERE, we'd better make sure that we dont get PHP code injection.
					// There is already a check on the PAINT DETAILS IF. But just in case.
					$matches = Array();
					preg_match('|(\w*)|', $group, $matches);
					eval('$complexType = Soap_Api_'.ucwords($matches[1]).'::getComplexElements();');

				?>
					<span style="color:#000000;font-size:12px;font-family:tahoma;font-weight:normal;">
						Description: <?php echo $func_info['func_desc'] ?>
					</span>
					<br /><br />

				<?php
				// Only display this if the function has a REQUEST format declared
				if (isset($complexType[$func_name])) {
					foreach ($complexType[$func_name] as $key => $args_info) {
						if ($key == 'arg_name') {
							echo $this->_printFuncInfo('Argument Name: ', NULL, 'margin-left:15px;').$this->_printFuncInfo(NULL, $args_info).'<br />';
						} else {
							echo $this->_printFuncInfo(NULL, $args_info['type'], 'margin-left:15px;').' '.$this->_printFuncInfo($key.': ').'<br />';
						}//end else
					}//end foreach
				}//end if
				echo $this->_printFuncInfo('Return type: ', NULL, 'margin-left:15px;').$this->_printFuncInfo(NULL, $func_info['return_type']).'<br /><br />';
					?>
				<a href="javascript:revealEntry('<?php echo sq_web_path('lib'); ?>','requestenvelope')" style="text-decoration:none;"><img name="expandrequestenvelope" src="<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/del.png" border="0" /><span style="border:0;font-size:13px;font-family:tahoma;color:#0099FF;font-weight:bold;"> View Request Envelope</span></a><br /><br />

					<div id="requestenvelope" style="display:;margin-top:10px;margin-left:12px;margin-bottom:10px;">
					<?php
					if (isset($complexType[$func_name])) {
						?>
						<span style="font-size:12px;font-family:tahoma;">
							The <?php echo $this->_placeholder('Placeholder') ?> text should be replaced with the correct data
						</span>
						<div style="width:90%;margin-top:10px;padding:20px;padding-top:10px;padding-bottom:10px;background:#EEEEEE;color:#000000;font-size:12px;font-family:tahoma;font-weight:normal;">
							<?php echo $this->_printRequestEnvelope($complexType[$func_name], $func_name); ?>
						</div>
					<?php
					}//end if
					?></div>

				<a href="javascript:revealEntry('<?php echo sq_web_path('lib'); ?>','returnenvelope')" style="text-decoration:none;"><img name="expandreturnenvelope" src="<?php echo sq_web_path('lib'); ?>/web/images/icons/internal_message/del.png" border="0" /><span style="border:0;font-size:13px;font-family:tahoma;color:#0099FF;font-weight:bold"> View Response Envelope</span></a><br />

						<div id="returnenvelope" style="display:;margin-top:10px;margin-left:12px;margin-bottom:10px;">
						<?php
						if (isset($complexType[$func_name.'Response'])) {
							?>
							<span style="font-size:12px;font-family:tahoma;">
								The <?php echo $this->_placeholder('Placeholder') ?> text should be replaced with the correct data
							</span>
							<div style="width:90%;margin-top:10px;padding:20px;padding-top:10px;padding-bottom:10px;background:#EEEEEE;color:#000000;font-size:12px;font-family:tahoma;font-weight:normal;">
								<?php echo $this->_printResponseEnvelope($complexType[$func_name.'Response'], $func_name); ?>
							</div>
							<?php
						}//end if

						?>
						</div>
						<?php

				} else {
					echo $this->_printFuncInfo('Service does not exists');
				}//end else

			}//end if

		}//end else

	}//end printBody()


	/**
	* This function check whether a particular function is enabled
	*
	* @return boolean
	* @access private
	*/
	private function getAvailableOperations($saveToObject=FALSE)
	{
		// Get all the APIs linked underneath
		$available_operations = Array();

		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
		$list_api = $this->attr('list_api');
		foreach ($children as $api_id => $type_info) {
			if (!isset($list_api[$api_id])) continue;
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$functions = $child->attr('function_list');

			foreach ($functions as $key => $value) {
				$functions[$key]	= Array (
										'class_name' => $type_code,
									  );
			}//end foreach

			$available_operations = array_merge($available_operations, $functions);
			if ($saveToObject) {
				$this->complex_elements 		= array_merge($this->complex_elements, $child->getComplexElements($functions));
				$this->complex_types			= array_merge($this->complex_types, $child->getComplexTypes($functions));
				$this->simple_restricted_types	= array_merge($this->simple_restricted_types, $child->getSimpleRestrictedTypes($functions));
				$this->function_list			= $available_operations;
			}//end if

		}//end foreach
		return $available_operations;

	}//end isFunctionEnable()


	/**
	* Print the title including the span
	*
	* @param string	$title		The Label in black color
	* @param string	$content	The content (arg_name) in blue color
	* @param string	$extra		Any extra CSS need to be added to the text
	*
	* @return string
	* @access public
	*/
	private function _printFuncInfo($title=NULL, $content=NULL, $extra=NULL)
	{
		$str = NULL;
		if (!is_null($title)) {
			$str = '<span style="font-size:12px;font-family:tahoma;font-weight:bold;'.$extra.'">'.$title.'</span>';
		} else if (!is_null($content)) {
			$ele_notes	= '';
			if (isset($this->simple_restricted_types[$content])) {
				$restricted_values	= $this->simple_restricted_types[$content]['enumeration'];
				sort($restricted_values);
				$ele_notes	= implode(', ', $restricted_values);
			}//end if
			$str = '<span style="color:#006699;font-size:12px;font-family:tahoma;font-weight:bold;'.$extra.'" title="'.$ele_notes.'">'.$content.'</span>';
		}//end if

		return $str;
	}//end _printFuncInfoTitle()


	/**
	* Print the Envelope of the SOAP Request
	*
	* @param array	$arg_info	The array contain information about the function
	* @param string	$func_name	The function name
	*
	* @return string
	* @access public
	*/
	private function _printRequestEnvelope($args_info, $func_name)
	{
		ob_start();
		?>
		<span>
		POST <?php echo $this->_placeholder($this->getUrl()) ?> HTTP/1.1<br />
		Host: <?php echo $this->_placeholder(sq_root_url($this->getUrl(),FALSE)) ?><br />
		Content-Type: <?php echo self::XML_CONTENT_TYPE ?><br />
		Content-Length: <?php echo $this->_placeholder('length') ?><br />
		</span>

		<br />

		<span>
			&lt;?xml version="1.0" encoding="utf-8" ?&gt;<br />
			&lt;soap:Envelope xmlns:xsi="<?php echo self::SOAP_XML_SCHEMA_INSTANCE ?>"
			xmlns:ns1="<?php echo $this->getUrl() ?>"
			xmlns:xsd="<?php echo self::SOAP_XML_SCHEMA_VERSION ?>"
			xmlns:soap="<?php echo self::SOAP_ENVELOP ?>"&gt;<br />
		</span>
		<div style="padding-left:10px">
			&lt;soap:Body&gt;<br />
				<div style="padding-left:10px">
					&lt;ns1:<?php echo $func_name ?>&gt;<br />
						<?php
							$this->printRequestResponseRecursive($args_info);
						?>
					&lt;/ns1:<?php echo $func_name ?>&gt;<br />
				</div>
			&lt;/soap:Body&gt;<br />
		</div>
		&lt;/soap:Envelope&gt;<br />
		<br />
		<?php
		$content = ob_get_contents();
		ob_end_clean();

		return $content;

	}//end _printRequestEnvelope()


	/**
	* This function print the complex types and simple types for the request or response
	*
	* @return void
	* @access private
	*/
	private function printRequestResponseRecursive($args_info)
	{
		foreach ($args_info as $key => $arg_info) {
			$io_type	= (is_array($arg_info)) ? $arg_info['type'] : '';
			// We have a recursive complex type here
			if ($io_type !== '' && isset($this->complex_elements[$io_type])) $io_type = '';
			$ele_notes	= '';
			if (isset($this->simple_restricted_types[$io_type])) {
				$restricted_values	= $this->simple_restricted_types[$io_type]['enumeration'];
				sort($restricted_values);

				$ele_notes	= implode(', ', $restricted_values);
			}//end if
			?><div style="padding-left:10px">
			&lt;<?php echo $key ?>&gt;<?php if (!empty($io_type)) echo $this->_placeholder($io_type, $ele_notes);

				// If we have a complex type inside
				if (!is_array($arg_info)) {
					if (isset($this->complex_elements[$arg_info])) {
						$this->printRequestResponseRecursive($this->complex_elements[$arg_info]);
					} else if (isset($this->simple_restricted_types[$arg_info])) {
						echo $this->_placeholder($arg_info);
					}//end if
				} else if (is_array($arg_info)) {
					// We are not going in here, because we don't want to print out any restricted type details.
					// It should be isset($this->simple_restricted_types[$arg_info['type']]) check instead
					//if (in_array($arg_info['type'], $this->simple_restricted_types)) {
					//	$this->printRequestResponseRecursive($this->simple_restricted_types[$arg_info['type']]);
					//} else
					if (isset($this->complex_elements[$arg_info['type']])) {
						$this->printRequestResponseRecursive($this->complex_elements[$arg_info['type']]);
					} else {

					}//end else

				}//end else
			?>&lt;/<?php echo $key ?>&gt;
			</div>
			<?php
		}//end foreach

	}//end printRequestResponseRecursive()


	/**
	* Print the Envelope of the SOAP Reponse
	*
	* @param array	$arg_info	The array contain information about the function
	* @param string	$func_name	The function name
	*
	* @return string
	* @access public
	*/
	private function _printResponseEnvelope($args_info, $func_name)
	{
		ob_start();
		?>
		<span>HTTP/1.0 200 OK<br />
			Date: <?php echo $this->_placeholder(date("D, j M Y h:i:s e")) ?><br />
			Server: <?php echo $this->_placeholder(sq_root_url($this->getUrl(),FALSE)) ?><br />
			Content-Type: <?php echo self::XML_CONTENT_TYPE ?><br />
			Content-Length: <?php echo $this->_placeholder('length') ?><br />
		</span>

		<br />

		<span>
			&lt;?xml version="1.0" encoding="utf-8" ?&gt;<br />
			&lt;soap:Envelope xmlns:xsi="<?php echo self::SOAP_XML_SCHEMA_INSTANCE ?>" <br />
			xmlns:ns1="<?php echo $this->getUrl() ?>"
			xmlns:xsd="<?php echo self::SOAP_XML_SCHEMA_VERSION ?>" xmlns:soap="<?php echo self::SOAP_ENVELOP ?>"&gt;<br />
		</span>

		<div style="padding-left:10px">
			&lt;soap:Body&gt;<br />
				<div style="padding-left:10px">
					&lt;ns1:<?php echo $func_name.'Response'; ?>&gt;<br />

						<?php $this->printRequestResponseRecursive($args_info); ?>

					&lt;/ns1:<?php echo $func_name.'Response'; ?>&gt;<br />
				</div>
			&lt;/soap:Body&gt;<br />
		</div>
			&lt;/soap:Envelope&gt;<br />
		<br />
		<?php

		$content = ob_get_contents();
		ob_end_clean();

		return $content;

	}//end _printResponseEnvelope()


	/**
	* Print a placeholder
	*
	* @param string	$name	The name to show as the placeholder
	*
	* @return string
	* @access private
	*/
	private function _placeholder($name='', $title='')
	{
		if (empty($name)) $name=' ';

		$placeholder_tag1 = '<span style="color:#003366;font-weight:bold;font-family:tahoma;font-size:12px;" title="'.$title.'">';
		$placeholder_tag2 = '</span>';

		return $placeholder_tag1.$name.$placeholder_tag2;

	}//end _placeholder()


	/**
	* This function get clean up the * from docComment
	*
	* @param string $comment	The docComment
	*
	* @return string
	* @access private
	*/
	private function cleanComment($comment)
	{
		$comment = str_replace('/*', '' , $comment);
		$comment = str_replace('*/', '' , $comment);
		$comment = str_replace('*', '' , $comment);

		return $comment;

	}//end if


	/**
	* Description: This function returns the WSDL for this soap server, including all available methods of
	* all APIs linked underneath.
	*
	* @return void
	* @access public
	*/
	private function getWSDL()
	{
		header(self::XML_CONTENT_TYPE);
		if (!file_exists($this->data_path.'/content_file.wsdl')) {
			$this->generateWSDL();
		}//end if
		$wsdl_content = file_get_contents($this->data_path.'/content_file.wsdl');
		if (!is_null($wsdl_content)) {
			echo $wsdl_content;
		}//end if

	}//end getWSDL()


	/**
	* This function construct the XML for the WSDL
	* Structure of the WSDL
	* <definition - Header stuff>
	* 		<types>
	* 				<schema>
	* 					<... all sorts of types ...>
	* 				</schema>
	* 		</types>
	* 		<message name="msg1">
	* 				<part name="parameters" element="tns:CreateAssetRequest"></part>
	* 		</message>
	* 		<message name="msg2">
	*				<part></part>
	* 		</message>
	* 		<portType>
	* 				<operation>
	* 						<input />
	* 						<output />
	* 				</operation>
	* 				...
	* 		</portType>
	* 		<binding>
	* 				<soap:binding>
	*				<operation>
	* 						<soap:operation>
	*						<input>
	*								<soap:body>
	* 						</input>
	* 						<output>
	* 								<soap:body>
	* 						</output>
	* 				</operation>
	* 		</binding>
	* 		<service>
	* 				<port>
	* 						<soap:address location="">
	* 				</port>
	* 		</service>
	* </definition>
	*
	* @return void
	* @access public
	*/
	private function getXML()
	{
		$wsdl = new DomDocument("1.0", 'utf-8');

		// Set all the header stuff
		$root = $wsdl->createElementNS('http://schemas.xmlsoap.org/wsdl/', 'wsdl:definitions');
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:s','http://www.w3.org/2001/XMLSchema');
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:tns', $this->getUrl());
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:soap',self::SCHEMA_SOAP);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:wsdl',self::SCHEMA_WSDL);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:soapenc',self::SOAP_SCHEMA_ENCODING);
		$root->setAttribute('targetNamespace', $this->getUrl());

			// Create the Types Node and set Name
			$types_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'types');
			$this->schema_node = $wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'schema');
			$this->schema_node->setAttribute('targetNamespace', $this->getUrl());
			// Lets set to to the instance variable and let it go.

			// Create the PortType Node and set Name
			$this->portType_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'portType');
			$this->portType_node->setAttribute('name', 'MatrixSOAPService');

			// Create the Binding Node and set Name
			$binding_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'binding');
			$binding_node->setAttribute('name', 'MatrixSOAPServiceBinding');
			$binding_node->setAttribute('type', 'tns:MatrixSOAPService');

				// Create the Soap Binding Node under Binding Node and set attributes
				$soap_binding_node = $wsdl->createElementNS(self::SCHEMA_SOAP, 'binding');
				$soap_binding_node->setAttribute('style', 'document');
				$soap_binding_node->setAttribute('transport', self::SCHEMA_SOAP_HTTP);

				// Append the Soap Binding Node to the Binding Node
				$binding_node->appendChild($soap_binding_node);

			// Assign soap binding node to the class instance var so we can append stuff to it
			$this->binding_node = $binding_node;	// Lets set it to the class instance variable and let it go.

		// Make the root element and the DomDoc obj be accessible anywhere within the class
		$this->root = $root;
		$this->wsdl = $wsdl;

		// Append Stuff (different types of OPERATIONs for both portTypes and Binding node) to all the nodes
		$this->getAPIsOperations();

		// Append the type to SCHEMA node
		$this->appendSimpleRestrictedTypes();
		$this->appendComplexTypes();
		$this->appendComplexElements();

		// Append SCHEMA node to TYPES node and then append TYPES node to ROOT node
		$types_node->appendChild($this->schema_node);
		$this->root->appendChild($types_node);

		// Create the MESSAGE(s) Node and set Name
		$io = Array ('input', 'output');
		foreach ($this->function_list as $func_name => $func_info) {
			foreach ($io as $element) {
				// Do the type before doing the messages
				$this->message_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'message');
				$this->message_node->setAttribute('name', $func_name.'Soap'.ucwords($element)
				);
					$part_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'part');
					$part_node->setAttribute('name', 'parameters');

					// May be we should never have a method that accept anytype of string, should all be complex type. IMPORTANT to rethink.
					$simpleType = FALSE;
					$message_io = $simpleType ? 's:'.$func_info['arg_type'] : 'tns:'.$func_name.(($element == 'output')?'Response':'');
					$part_node->setAttribute('element', $message_io);

				$this->message_node->appendChild($part_node);
				$this->root->appendChild($this->message_node);
			}//end foreach
		}//end foreach

		// Append all the major nodes to root node
		$this->root->appendChild($this->portType_node);
		$this->root->appendChild($binding_node);
		$this->appendService();

		// Append the root node to the DOM document
		$wsdl->appendChild($root);

		// Finally produce XML
		return $this->wsdl->saveXML();

	}//end getXML()


	/**
	* This function Append nodes to the XML Root element while traversing and looking for function in
	* each APIs linked underneath. Function information is based on PHPDoc comments, so the comments have
	* to be VERY accurate
	*
	* @return void
	* @access public
	*/
	public function getAPIsOperations()
	{
		// Get all the APIs linked underneath

		$this->getAvailableOperations(TRUE);

		// Loop through the list of function names to get the docComment for each of them,
		// and append a node to the XML tree right away
		foreach ($this->function_list as $func_name => $func_info) {
				// Get the Reflection class to get the info about each function
				$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', $func_info['class_name']))));

				if (method_exists($func_info['class_name'], $func_name)) {
					$input_output = Array ('input', 'output');

					// Adding OPERATION NODE to BINDING NODE
					$operation_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, 'operation');
					$operation_node->setAttribute('name', $func_name);
					$sub_operation_node = $this->wsdl->createElementNS(self::SCHEMA_SOAP, 'operation');
					$sub_operation_node->setAttribute('soapAction', '');

					// Get a copy of the Node here since we don't need the io_node here.
					$operation_node_2 = clone $operation_node;

					$operation_node->appendChild($sub_operation_node);
					// Adding IO node to each of the operations inside SOAP BINDING NODE.
					foreach ($input_output as $element) {
						$io_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, $element);
						$body_node = $this->wsdl->createElementNS(self::SCHEMA_SOAP, 'body');
						$body_node->setAttribute('use', 'literal');
						$io_node->appendChild($body_node);
						$operation_node->appendChild($io_node);
					}//end foreach

					$this->binding_node->appendChild($operation_node);

					// Adding OPERATION NODE to PORTTYPE NODE using $operation_node_2
					foreach ($input_output as $element) {
						$io_message_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, $element);
						$io_message_node->setAttribute('message', 'tns:'.$func_name.'Soap'.ucwords($element));
						$operation_node_2->appendChild($io_message_node);
					}//end foreach
					$this->portType_node->appendChild($operation_node_2);

					// Get the PHPDoc comment
					$method = $class->getMethod($func_name);
					$doc_comment = $method->getDocComment();

					// Get the array of information about the function including arg_name, arg_type, return_type, etc...
					$func_args_info = $this->getFuncDocInfo($doc_comment);

					// Add this information to the global function_list array.
					$this->function_list[$func_name] = array_merge($func_info, $func_args_info);

				}//end if

		}//end foreach

	}//end getAPIsOperations()


	/**
	* This function get all the information of a function based on the function's docComment
	*
	* @param string $doc_comment	The docComment of a particular function, then regex it to get the info
	*
	* @return array
	* @access private
	*/
	private function getFuncDocInfo($doc_comment)
	{
		$matches = Array();

		preg_match_all('|@param\s+(\w+)\s+(\&)?\$(\w+)|', $doc_comment, $matches);

		$func_args_info	= Array (
							'arg_type'	=> isset($matches[1][0]) ? $matches[1][0] : '',
							'arg_ref'	=> isset($matches[2][0]) ? $matches[2][0] : '',
							'arg_name'	=> isset($matches[3][0]) ? $matches[3][0] : '',
						  );

		$matches_0 = Array();
		preg_match_all('|Description:\s+((\w+\s)*)|', $doc_comment, $matches_0);
		$func_args_info['func_desc'] = isset($matches_0[1][0]) ? trim($matches_0[1][0]) : '';

		$matches_2 = Array();
		preg_match('|@return\s+(\w+)|', $doc_comment, $matches_2);
		$func_args_info['return_type']	= isset($matches_2[1]) ? trim($matches_2[1]) : '' ;

		return $func_args_info;

	}//end getFuncDocInfo()


	/**
	* This function append the Complex Elements defined in each API to the TYPES nodes. The complex elements are required
	* to pass multiple argument to a function.
	*
	* @return void
	* @access public
	*/
	private function appendComplexElements()
	{
		// Loop through the complex types came from all the APIs.
		foreach ($this->complex_elements as $func_io => $sub_args_info) {

				// Hiding unknown complex elements
				$func_name_safe = str_replace('Response', '', $func_io);
				if (!array_key_exists($func_name_safe, $this->function_list)) continue;

				// Specify the REQUEST - RESPONSE for each function, point them to the definition of the complex type.
				$element_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'element');
				$element_node->setAttribute('name', $func_io);

					$complex_main_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'complexType');
					// If we are declaring complex types which are needed for the input, we need to set the complex type name as we are not inside <element>

						$sequence_main_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'sequence');

							// It is raw type, DAMN
							foreach ($sub_args_info as $sub_arg_name => $info) {

									$subelement_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'element');
									$subelement_node->setAttribute('name', $sub_arg_name);
									if (strtolower($info['type']) == 'any') {
										$sub_complex_node	= $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'complexType');
										$sub_sequence_node	= $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'sequence');
										if ($info['mixed']) {
											$sub_complex_node->setAttribute('mixed', $info['mixed'] ? "true" : "false");
										}//end if
										$any_node	= $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'any');
										$sub_sequence_node->appendChild($any_node);
										$sub_complex_node->appendChild($sub_sequence_node);
										$subelement_node->appendChild($sub_complex_node);
									} else {
										$subelement_node->setAttribute('type', (!in_array($info['type'], $this->simple_types) ? 'tns:' : 's:' ).$info['type']);
									}
									$subelement_node->setAttribute('minOccurs', (!is_array($info)) ? '1': $info['min']);
									$subelement_node->setAttribute('maxOccurs', (!is_array($info)) ? '1': $info['max']);
									if ($info['nillable']) {
										$subelement_node->setAttribute('nillable', strtolower((string) $info['nillable']));
									}//end if

									$sequence_main_node->appendChild($subelement_node);
							}//end foreach

					$complex_main_node->appendChild($sequence_main_node);
				$element_node->appendChild($complex_main_node);
				// end REQUEST - RESPONSE specification

				// This is the REQUEST - RESPONSE specification
				// If we are declaring a complex type, we dont want to be inside the element node.
				$this->schema_node->appendChild($element_node);

		}//end foreach

	}//end appendComplexElements()


	/**
	* This function append the Complex Types defined in each API to the TYPES nodes. The complex types are required
	* to pass multiple argument to a function.
	*
	* @return void
	* @access public
	*/
	private function appendComplexTypes()
	{
		// Loop through the complex types came from all the APIs.
		foreach ($this->complex_types as $func_io => $sub_args_info) {
			// Definition of the complex type (whether it is a string, int or float)
			$complex_type_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'complexType');
			$complex_type_node->setAttribute('name', $func_io);
			if (is_array($sub_args_info)) {
				$sequence_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'sequence');
					foreach ($sub_args_info as $sub_arg_name => $info) {
						if ($sub_arg_name == 'arg_name') continue;
							$subelement_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'element');
							$subelement_node->setAttribute('name', $sub_arg_name);
							// We only add type attribute for most complex type attribute, if the complex type does not have a type
							// very likely that it has a mixed attribute and return 'any'
							if (!empty($info['type'])) {
									$subelement_node->setAttribute('type', (!in_array($info['type'], $this->simple_types) ? 'tns:' : 's:' ).$info['type']);
							}//end if
							$subelement_node->setAttribute('minOccurs', $info['min']);
							$subelement_node->setAttribute('maxOccurs', $info['max']);
							if ($info['nillable']) {
								$subelement_node->setAttribute('nillable', strtolower((string) $info['nillable']));
							}//end if
							if ($info['mixed']) {
								$subelement_node->setAttribute('mixed', strtolower((string) $info['mixed']));
							}//end if

							$sequence_node->appendChild($subelement_node);
					}//end foreach
				$complex_type_node->appendChild($sequence_node);
				// End complex type definition
			} else {
				$complex_type_node->setAttribute('type', 'tns:'.$sub_args_info);
			}//end else

			// This is the complex type definition
			$this->schema_node->appendChild($complex_type_node);
		}//end foreach

	}//end appendComplexTypes()


	/**
	* This function append the Complex Types defined in each API to the TYPES nodes. The complex types are required
	* to pass multiple argument to a function.
	*
	* @return void
	* @access public
	*/
	private function appendSimpleRestrictedTypes()
	{
		// Loop through the complex types came from all the APIs.
		foreach ($this->simple_restricted_types as $simple_restricted_type_name => $sub_args_info) {
			// Definition of the simple type (whether it is a string, int or float)
			$simple_restricted_type_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'simpleType');
			$simple_restricted_type_node->setAttribute('name', $simple_restricted_type_name);
			if (is_array($sub_args_info)) {
				$restriction_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'restriction');
				$restriction_base = in_array($sub_args_info['restriction_base'], $this->simple_types) ? 's:'.$sub_args_info['restriction_base'] : $sub_args_info['restriction_base'];
				$restriction_node->setAttribute('base', $restriction_base);
				if (isset($sub_args_info['enumeration']) && is_array($sub_args_info['enumeration'])) {
					foreach ($sub_args_info['enumeration'] as $enum_val) {
						$enum_node	= $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'enumeration');
						$enum_node->setAttribute('value', $enum_val);
						$restriction_node->appendChild($enum_node);
					}//end foreach
				} else if (isset($sub_args_info['pattern'])) {
					$pattern_node	= $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'pattern');
					$pattern_node->setAttribute('value', $sub_args_info['pattern']);
					$restriction_node->appendChild($pattern_node);
				}//end else
				$simple_restricted_type_node->appendChild($restriction_node);
				// End complex type definition
			} else {
				$simple_restricted_type_node->setAttribute('type', 'tns:'.$sub_args_info);
			}//end else

			// This is the complex type definition
			$this->schema_node->appendChild($simple_restricted_type_node);

		}//end foreach

	}//end appendSimpleRestrictedTypes()


	/**
	* This function add the SERVICE element to the end of the WSDL
	*
	* @return void
	* @access public
	*/
	private function appendService()
	{
		// Create a new service node
		$service_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, 'service');
		$service_node->setAttribute('name', 'MatrixWebServices');

			$port_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, 'port');
			$port_node->setAttribute('name', 'MatrixWebServicesPort');
			$port_node->setAttribute('binding', 'tns:MatrixSOAPServiceBinding');

				$address_node = $this->wsdl->createElementNS(self::SCHEMA_SOAP, 'address');
				$address_node->setAttribute('location', $this->getUrl().'?wsdl');

			$port_node->appendChild($address_node);
		$service_node->appendChild($port_node);

		// Append SERVICE node to root
		$this->root->appendChild($service_node);

	}//end appendComplexTypes()


	/**
	* Regerate WSDL
	*/
	public function generateWSDL()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$wsdl_content = $this->getXML();
		create_directory($this->data_path);
		return string_to_file($wsdl_content, $this->data_path.'/content_file.wsdl');

	}//end generateWSDL()

}//end class
?>
