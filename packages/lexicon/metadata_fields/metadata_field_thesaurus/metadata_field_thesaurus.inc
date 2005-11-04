<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_thesaurus.inc,v 1.9 2005/11/04 06:02:04 arailean Exp $
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
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Metadata_Field_Thesaurus extends Metadata_Field
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Field_Thesaurus($assetid=0)
	{
		$this->Metadata_Field($assetid);

	}//end constructor


	/**
	* Returns a reference to the parent thesaurus asset
	*
	* @return object
	* @access public
	*/
	function &getThesaurusRef()
	{
		$th_id = $this->attr('thesaurus_assetid');
		if ($th_id == -1) return false;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($th_id);

	}//end getThesaurusRef()


}//end class

?>
