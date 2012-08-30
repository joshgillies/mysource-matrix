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
* $Id: bulkmail_post_office.inc,v 1.24 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_constant.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once 'Mail.php';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';
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
* @version $Revision: 1.24 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

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
	function _getName($short_name=FALSE)
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
														'exclusive'	=> FALSE,
													   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'bulkmail_job'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'bulkmail_job'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
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
		if ($result !== TRUE) return $result;

		if (($minor instanceof Bulkmail_Job) && ($link_type != SQ_LINK_NOTICE)) {
			// bulkmail job can only link to one bulkmail post office
			$bmail_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'bulkmail_post_office', FALSE, 'minor');
			if (!empty($bmail_link)) {
				return translate('bm_warning_one_post_office_for_job');
			}
		}

		return TRUE;

	}//end canCreateLink()


	/**
	* Ensures that bulkmail jobs can be moved from one post office to another
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param object	&$old_major	the major asset that we are being moved from
	* @param string	$link_type	the type of link this is
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canMoveLink(&$minor, &$old_major, $link_type)
	{
		if (($minor instanceof Bulkmail_Job) && ($old_major instanceof Bulkmail_Post_Office) && ($link_type != SQ_LINK_NOTICE)) {
			 // tolerate one existing bmail_link if we are moving jobs from one post office to another
			$bmail_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'bulkmail_post_office', FALSE, 'minor');
			if (count($bmail_link) > 1) {
				return translate('bm_warning_one_post_office_for_job');
			}
		} else {
			// if we aren't moving from a post office, the usual check applies
			return parent::canMoveLink($minor, $old_major, $link_type);
		}

		return TRUE;

	}//end canMoveLink()

	
	/**
	* Use tools config file for sendmail path
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed string|NULL
	* @access public
	*/
	public function attr($name)
	{
		$value = parent::attr($name);
		if($name === 'server_details' ) {
		    $driver = array_get_index($value, 'driver', '');
		    if($driver === 'sendmail') {
			if(SQ_TOOL_SENDMAIL_PATH !== '') {
			    $value['sendmail_path'] = SQ_TOOL_SENDMAIL_PATH;
			}
			 if(SQ_TOOL_SENDMAIL_ARGS !== '') {
			    $value['sendmail_args'] = SQ_TOOL_SENDMAIL_ARGS;
			}
		    }
		}
		
		return $value;

	}//end attr()

}//end class
?>
