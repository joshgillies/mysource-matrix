<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: soap_wsdl_message_edit_fns.inc,v 1.5 2006/12/07 04:06:29 bcaldwell Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Message_Edit_Fns extends SOAP_WSDL_Component_Edit_Fns
{


	/**
	*/
	function paintMessageStructure(&$asset, &$o, $prefix)
	{
		$type_links = $asset->schema->getTypeLinks($asset->id);
		foreach ($type_links as $type_link) {
			// paint type interfaces
			$type_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($type_link['minorid']);

			$container = Array();
			$type_asset->processBackendInterface($asset, $o, $_POST[$prefix], $container);
			bam($container);

			$type_asset->paintBackendInterface($asset, $o, $prefix, $container);
		}

	}//end paintMessageStructure()


	/**
	*/
	function processMessageStructure(&$asset, &$o, $prefix)
	{
		/*$operation_name = $asset->schema->getComponentName($asset->id, SQ_SOAP_WSDL_OPERATION);
		$message_name = $asset->schema->getComponentName($asset->id, SQ_SOAP_WSDL_MESSAGE);

		$result = $asset->schema->callOperation('port', $operation_name, $_POST[$message_name]);
		bam($result);*/

	}//end processMessageStructure()


}//end class

?>