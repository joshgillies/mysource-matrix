<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source	   |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this	   |
* | Module if you have the written consent of Squiz.				   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_core.inc,v 1.5 2008/05/26 00:48:58 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* SOAP API Core
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Core extends Soap_Api
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array(
				'createAsset'		=> '1',
				'deleteAsset'		=> '1',
				'findChildren'		=> '1',
				'findLinks'			=> '1',
				'newAssetLink'		=> '1',
				'duplicateAsset'	=> '1',
				'changeStatus'		=> '1',
				'findStatus'		=> '1',
				'changeAttrValue'	=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This function will create an asset of a specific type under a specific location
	*
    * @param array  $link_info	information used to create the initial link
    * <pre>
    * Array (
	*		 'type_code'	 => [type code of new asset],
	*		 'name'			 => [name for new asset],
	*		 'parentid'		 => [parentid of the new parent],
    *        'link_type'     => SQ_LINK_?,
    *        'value'         => [link value],
    *        'sort_order'    => [link sort order],
    *        'is_dependant'  => [0|1],
    *        'is_exclusive'  => [0|1]
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function createAsset($request)
	{
		$link_info = (Array) $request;
		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_info['parentid']);
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($link_info['type_code']);
		$object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $link_info['type_code'])));
		$asset = new $object_name();
		$asset->setAttrValue('name', $link_info['name']);

		$link_info['asset']	= $parent_asset;
		$linkid = $asset->create($link_info);
		if ($linkid) {
			return Array (
					'result' => 'Asset was created successfully. Link id #'.$linkid,
				   );
		}//end if

		throw new SoapFault('Server', 'Unable to create asset');

	}//end createAsset()


	/**
	* deleteAsset
	*
    * @param string  $assetid  the id of the asset being deleted
	*
	* @return void
	* @access public
	*/
	function deleteAsset($assetid)
	{
		// THIS FUNCTION HAS NOT BEEN FULLY IMPLEMENTED, Therefore it is disabled.
		//$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		//$minor_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid->assetid->assetid);
		//if ($GLOBALS['SQ_SYSTEM']->am->createAssetLink($trash_folder, $minor_asset, SQ_LINK_TYPE_1)) {
		//	return 'ALL GOOD';
		//} else {
		//	return 'FAILED';
		//}//end else if

	}//end deleteAsset()


	/**
	* Description: This function returns all the children of a specific asset
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'assetid'			=> [assetid of node],
	*		'type_code'			=> [type code of new asset],
	*		'strict_type_code'	=> [strict type code or not],
	*		'dependant'			=> [dependant or not],
    *		'link_value'		=> [link value],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function findChildren($asset_info_request)
	{
		// THIS FUNCTION HAS NOT BEEN FULLY IMPLEMENTED.
		//$asset_info = (Array) $asset_info_request->asset_info;
		//$GLOBALS['SQ_SYSTEM']->am->getChildren($asset_info['assetid'], $asset_info['type_code'], $asset_info['strict_type_code'], $asset_info['dependant'], NULL, NULL, TRUE, NULL, NULL, TRUE, $asset_info['link_value']);

	}//end findChildren()


	/**
	* findLinks
	*
	* @return void
	* @access public
	*/
	function findLinks($assetid)
	{


	}//end findLinks()


	/**
	* newAssetLink
	*
	* @return void
	* @access public
	*/
	function newAssetLink($assetid)
	{


	}//end newAssetLink()


	/**
	* duplicateAsset
	*
	* @return void
	* @access public
	*/
	function duplicateAsset($assetid)
	{


	}//end duplicateAsset()


	/**
	* changeStatus
	*
	* @return void
	* @access public
	*/
	function changeStatus($assetid)
	{


	}//end changeStatus()


	/**
	* findStatus
	*
	* @return void
	* @access public
	*/
	function findStatus()
	{


	}//end findStatus()


	/**
	* changeAttrValue
	*
	* @return void
	* @access public
	*/
	function changeAttrValue($assetid)
	{


	}//end changeAttrValue()


	/**
	* getComplexType
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$string_non_optional = self::getArgType('string', 1, 1);
		$string_optional = self::getArgType('string', 0, 1);
		$int_optional = self::getArgType('int', 0, 1);

		$complex_types	= Array (
							'createAsset'	=> Array (
												'createAsset'	=> Array (
									    									'type_code'     => $string_non_optional,
																			'name'          => $string_non_optional,
																			'parentid'      => $string_non_optional,
																			'link_type'     => $string_non_optional,
																			'value'         => $string_optional,
																			'sort_order'    => $int_optional,
																			'is_dependant'  => $string_optional,
																			'is_exclusive'  => $string_optional,
																		  ),
												'createAssetResponse'	=> Array (
																			'result'		=> $string_optional,
																		   ),
											   ),
							'deleteAsset'	=> Array (
												'deleteAsset'	=> Array (
																			'assetid'		=> $string_non_optional,
																		   ),
												'deleteAssetResponse'	=> Array (
																			'result'		=> $string_optional,
																		   ),
											   ),
							'findChildren'	=> Array (
												'findChildren'	=> Array (
																			'assetid'			=> $string_non_optional,
																			'type_code'			=> $string_optional,
																			'strict_type_code'	=> $string_optional,
																			'dependant'			=> $string_optional,
																			'link_value'		=> $string_optional,
																		   ),
												'findChildrenResponse'	=> Array (
																			'result'		=> $string_optional,
																		   ),
											   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;


	}//end getComplexTypes();

}//end class
?>
