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
* $Id: bodycopy_div.inc,v 1.2.2.3 2004/03/02 18:35:51 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container.inc';

/**
* Bodycopy_Div
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Div extends Bodycopy_Container
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy_Div($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Bodycopy_Container($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link		information used to create the initial link
	* @param array	$bc_data	bodycopy data submitted to help create this table
	*
	* @return mixed int or false
	* @access public
	*/
	function create(&$link, $bc_data)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a BodyCopy DIV with a blank name', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// set the name to what they entered - if they entered anything
		if (isset($bc_data['attributes']['identifier']) && trim($bc_data['attributes']['identifier']) != '') {
			$this->setAttrValue('name', $bc_data['attributes']['identifier']);
		}

		// create a wysiwyg content type
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('content_type_wysiwyg');
		$wysiwyg =& new Content_Type_Wysiwyg();
		$div_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'div_contents', 'sort_order' => 1, 'dependant' => 1, 'exclusive' => 1);
		$wysiwyg->setAttrValue('name', 'DIV Content');
		if (!$wysiwyg->create($div_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_2 => Array('content_type' => Array('card' => '1', 'exclusive' => true)),
					 SQ_LINK_NOTICE => Array('asset' => Array('card' => 'M', 'exclusive' => false))
					 );

	}//end _getAllowedLinks()


}//end class

?>