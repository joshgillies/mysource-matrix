<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: soap_wsdl_message_edit_fns.inc,v 1.3 2006/11/02 00:31:21 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component_edit_fns.inc';


/**
* SOAP_WSDL_Message_Edit_Fns
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Message_Edit_Fns extends SOAP_WSDL_Component_Edit_Fns
{


	/**
	*/
	function paintMessageStructure(&$asset, &$o, $prefix)
	{
		echo '<table>';

		$message_name = $asset->schema->getComponentName($asset->id);
		$message_wsdl = $asset->schema->getComponentWsdl($asset->id);

		foreach ($message_wsdl as $param_name => $param_wsdl) {

			$id_parts = Array();
			$param_type = SQ_SOAP_WSDL_TYPE;

			$id_parts[SQ_SOAP_WSDL_NAME]		= array_get_index($param_wsdl, 'name');
			$id_parts[SQ_SOAP_WSDL_TYPE]		= array_get_index($param_wsdl, 'type');
			$id_parts[SQ_SOAP_WSDL_ELEMENT]		= array_get_index($param_wsdl, 'element');
			$id_parts[SQ_SOAP_WSDL_NAMESPACE]	= array_get_index($param_wsdl, 'namespace');

			if (!is_null($id_parts[SQ_SOAP_WSDL_ELEMENT])) {
				// extract element name and namespace from qname
				$qname =& new QName($id_parts[SQ_SOAP_WSDL_ELEMENT]);

				$id_parts[SQ_SOAP_WSDL_ELEMENT]		= $qname->name;
				$id_parts[SQ_SOAP_WSDL_NAMESPACE]	= $qname->ns;

				$param_type = SQ_SOAP_WSDL_ELEMENT;
			} else {
				unset($id_parts[SQ_SOAP_WSDL_ELEMENT]);
			}
			$assetid = $asset->schema->getComponentAssetId($id_parts, $param_type);

			$type_asset =& $GLOBALS['SQ_SYSTEM']->am->GetAsset($assetid);
			$type_asset->paintBackendInterface($message_name );

		}//end foreach

		echo '</table><hr />';

	}//end paintMessageStructure()


	/**
	*/
	function processMessageStructure(&$asset, &$o, $prefix)
	{
		$operation_name = $asset->schema->getComponentName($asset->id, SQ_SOAP_WSDL_OPERATION);
		$message_name = $asset->schema->getComponentName($asset->id, SQ_SOAP_WSDL_MESSAGE);

		$result = $asset->schema->callOperation('port', $operation_name, $_POST[$message_name]);
		bam($result);

	}//end processMessageStructure()


}//end class

?>