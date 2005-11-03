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
* $Id: bulkmail_post_office.inc,v 1.9 2005/11/03 01:08:02 rong Exp $
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
* @version $Revision: 1.9 $
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
				return 'Bulkmail Job can only be linked under one Bulkmail Post Office';
			}
		}

		return true;

	}//end canCreateLink()


	/**
	* Ensures that bulkmail jobs can only be moved from one post office to another
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
		if (is_a($minor, 'bulkmail_job') && is_a($old_major, 'bulkmail_post_office') && ($link_type != SQ_LINK_NOTICE)) {
			// if we are moving from a post office, we can tolerate one existing
			// network link - the one being moved from
			$bmail_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'bulkmail_post_office', false, 'minor');
			if (count($bmail_link) > 1) {
				return 'Bulkmail Job can only be linked under one Bulkmail Post Office';
			}
		} else {
			// if we aren't moving from a post office, the usual check applies
			return parent::canMoveLink($minor, $old_major, $link_type);
		}

		return true;

	}//end canMoveLink()


	/**
	* Sends the bulkmail message using given PEAR mail driver params
	*
	* @param mixed	&$asset		the bulkmail job
	* @param mixed	$recipient	an array or a string with comma separated recipients.
	*
	* @return boolean
	* @access public
	*/
	function sendPreviewMail(&$asset, $recipient)
	{
		if (is_null($asset->getPostOffice())) {
			// unable to create mail object
			trigger_localised_error('BML0005', E_USER_NOTICE, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'), $asset->name, $asset->id);
			return false;
		}
		// Create the mail object using server_details as parameter
		$server_details = $asset->getServerDetails(); // TODO: change
		$driver = array_get_index($server_details, 'driver', '');
		$mail_object =& Mail::factory($driver, $server_details);
		// integrate PEAR error object's message with matrix's error reporting system
		if (PEAR::isError($mail_object)) {
			// unable to create mail object
			trigger_localised_error('BML0001', E_USER_WARNING, $mail_object->getMessage());
			return false;
		}
		$content_details = $asset->attr('content_details');
		// cache the content as the current user/pre-selected user depending on the content generation method
		$asset->cacheContent($asset, $content_details, $GLOBALS['SQ_SYSTEM']->currentUserId());
		$content = $this->generateBmailContent($asset, $content_details);


		// We might do this later, texttohtml of some sort
		$mime =& new Mail_mime();
		//$mime->setTxtBody($textMessage);
		$mime->setHtmlBody($content);
		$body = @$mime->get();

		$header_details = $asset->getHeaderDetails();
		// add 'Preview' to the subject of the email
		if (isset($header_details['Subject'])) {
			$header_details['Subject'] = $header_details['Subject'].' (Preview)';
		}
		$headers = @$mime->headers($header_details);
		$status = $mail_object->send($recipient, $headers, $body);
		if (PEAR::isError($status)) {
			// unable to send bulkmail
			trigger_localised_error('BML0002', E_USER_WARNING, $status->getMessage());
			return false;
		}
		return true;

	}//end sendPreviewMail()


	/**
	* Generates the new preview log message of this preview bmail
	*
	* @param array	$recipient	all the recipients of this preview email
	*
	* @return string
	* @access public
	*/
	function generateLog($recipient)
	{
		$msg = translate('bulkmail_preview_message');
		$who = implode(', ', $recipient).'<br /> on ';
		$when = date('r');

		return $msg.'&nbsp; '.$who.$when;
	}//end generateLog()


	/**
	* Gets status directly from the database, as attr() doesn't return the updated value
	*
	* @param int	$assetid	$this->id, since we are calling this function statically
	*
	* @return string
	* @access public
	*/
	function getStatusDB($assetid)
	{
		$sql = 'SELECT status ';
		$sql .= 'FROM sq_ast ';
		$sql .= "WHERE assetid = '".$assetid."'";
		$result = $GLOBALS['SQ_SYSTEM']->db->getOne($sql);
		assert_valid_db_result($result);

		return $result;

	}//end getStatusDB()


	/**
	* Checks if this post office has any child that is a running live job
	*
	* To be used in edit functions to disable writing to the attributes
	*
	* @return mixed int|boolean
	* @access public
	*/
	function hasLiveChild()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$strict = true;

		// get all bulkmail post offices and jobs that are currently in the trash folder
		$trash_folder = &$am->getSystemAsset('trash_folder');
		$in_trash = $am->getChildren($trash_folder->id, 'bulkmail_job', $strict);

		// get all jobs linked under this post office, which are not in trash
		$jobs = $am->getChildren($this->id, 'bulkmail_job', $strict);
		$jobs = array_diff_assoc($jobs, $in_trash);

		// get live job (only one live job at any given point of time)
		foreach ($jobs as $job_assetid => $type_code) {
			$status = Bulkmail_Post_Office::getStatusDB($job_assetid);
			if ($status == SQ_STATUS_LIVE) return $job_assetid;
		}

		return false;

	}//end hasLiveChild()


}//end class
?>
