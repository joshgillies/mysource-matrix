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
* $Id: bulkmail_post_office.inc,v 1.16 2005/11/14 23:59:38 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_constant.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once 'Mail.php';
@require_once 'Mail/mime.php';


/**
* Bulkmail_Post_Office
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bulkmail_Post_Office($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		// ensure that each job is only linked to one post office in canCreateLink and canMoveLink
		// see the relationship between site_network and site for more info
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'bulkmail_job'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> false,
													   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'bulkmail_job'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> false,
													   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'bulkmail_job'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> false,
													   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Ensures that a bulkmail job can only be linked to one unique post office
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		$result = parent::canCreateLink($minor, $link_type, $exclusive);
		if ($result !== true) return $result;

		if (is_a($minor, 'bulkmail_job') && ($link_type != SQ_LINK_NOTICE)) {
			// bulkmail job can only be in a single bulkmail post office
			$network_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'bulkmail_post_office', false, 'minor');
			if (!empty($network_link)) {
				return translate('bm_warning_one_post_office_for_job');
			}
		}

		return true;

	}//end canCreateLink()


}//end class
?>
