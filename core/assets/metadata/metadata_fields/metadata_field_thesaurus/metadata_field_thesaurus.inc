<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_thesaurus.inc,v 1.5 2012/08/30 01:09:09 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/metadata/metadata_field/metadata_field.inc';

/**
* Metadata_Field_Select
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Thesaurus extends Metadata_Field
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a reference to the parent thesaurus asset
	*
	* @return mixed Thesaurus|bool
	* @access public
	*/
	public function getThesaurusRef()
	{
		$th_id = $this->attr('thesaurus_assetid');
		$false = FALSE;
		if (empty($th_id)) return $false;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($th_id);

	}//end getThesaurusRef()


}//end class

?>
