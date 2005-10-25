<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'Mail.php';

define('BML_JOB_STATE_NEW',      0);
define('BML_JOB_STATE_NOT_LIVE', 1);
define('BML_JOB_STATE_RUNNING',  2);
define('BML_JOB_STATE_PAUSED',   3);
define('BML_JOB_STATE_FINISHED', 4);

/**
* Bulkmail_Post_Office
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.1 $
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
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		// ensure that each job is only linked to one post office in canCreateLink and canMoveLink
		// see the relationship between site_network and site for more info
		return Array(
				SQ_LINK_TYPE_1 => Array('bulkmail_job' => Array('card' => 'M', 'exclusive' => false)),
				SQ_LINK_TYPE_2 => Array('bulkmail_job' => Array('card' => 'M', 'exclusive' => false)),
				SQ_LINK_NOTICE => Array('bulkmail_job' => Array('card' => 'M', 'exclusive' => false)),
			   );

	}//end _getAllowedLinks()


	/**
	* Ensures that a bulkmail job can only be linked to one unique post office
	*
	* @param object	$minor		the minor asset that we are linking to
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
	* Generates the html bulkmail content based on the user and content generation type
	*
	* @param object	&$asset				the asset to which we belong
	* @param array	$content_details	information related to content generation
	* @param int	$user_id			the recipient of this email
	*
	* @return string
	* @access public
	*/
	function generateBmailContent(&$job, $content_details, $user_id=0)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$type = array_get_index($content_details, 'type', 'group');
		$cache = false;

		// check if the content is already cached for group-based
		if ($type == 'group' && file_exists($this->data_path.'/.data/cache/content_file.html')) {
			$content = file_to_string($this->data_path.'/.data/cache/content_file.html');
		}

		// get content page asset if necessary
		if (empty($content)) {
			if (isset($content_details['content_page'])) {
				$page = $am->getAsset($content_details['content_page']);
				//echo " LOADED Content Page (Id #$content_details[content_page]) from am\n";
			} else {
				// error: no content page set
				trigger_localised_error('BML0003', E_USER_NOTICE, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($job->type(), 'name'), $job->name, $job->id);
				exit;
			}
		}

		// setup user according to generation type
		switch ($type) {
			case 'group':
				// set user for keyword replacement if cached, do not need bulkmail_user to generate content
				if (!empty($content)) {
					if($user_id) $user = $am->getAsset($user_id);
					//echo " LOADED User (Id #$user_id)\n";
					break;
				}
				if (isset($content_details['bulkmail_user'])) {
					$user = $am->getAsset($content_details['bulkmail_user']);
					$cache = true;
					//echo " LOADED Bulkmail User (Id #$content_details[bulkmail_user])\n";
				} else {
					// error: no bulkmail user set
					trigger_localised_error('BML0004', E_USER_NOTICE, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($job->type(), 'name'), $job->name, $job->id);
					exit;
				}
				break;

			case 'user':
				if ($user_id) {
					$user = $am->getAsset($user_id);
					//echo " LOADED User (Id #$user_id)\n";
				}
				break;
		}

		// skip if content is already generated for group-based
		// always execute this part for per-user generation
		if(empty($content)){
			//echo "create content ONCE\n";

			// stores the session data if generating content for preview email
			if (!$user_id) $old_session = $_SESSION;

			// login as this user and print page content
			if(!$GLOBALS['SQ_SYSTEM']->setCurrentUser($user)){
				// happens when the user is not live or in trash
				trigger_localised_error('BML0006', E_USER_NOTICE, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($user->type(), 'name'), $user->name, $user->id);
				return '';
			}

			ob_start();
				// dont want any notice/warning here
				@$page->printFrontend();
				$content = ob_get_contents();
			ob_end_clean();

			// restore the session data so that current user doesnt get invalid login key error for preview email
			if (!$user_id) {
				$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
				$_SESSION = $old_session;
			}

			// cache html content for group-based
			if($type == 'group') {
				if ($cache) create_directory($this->data_path.'/.data/cache');
				if (!string_to_file($content, $this->data_path.'/.data/cache/content_file.html')) {
					echo 'error: cannot cache content\n';
					return '';
				}
			}
		}//end if(empty($content))

		// keyword replacement
		if ($user_id && !is_null($user)) {
			// otherwise will replace keyword for bmail_user instead of this user
			if ($cache) $user = $am->getAsset($user_id);
			$content = $this->replaceContentKeyword($content, $user);
		}

		return $content;

	}//end generateBmailContent()


	/**
	* Keyword replacement for customised email content of a user
	*
	* @param string	$content	page content before keyword replacement
	* @param int	$user		personalise content for this user
	*
	* @return string
	* @access public
	*/
	function replaceContentKeyword($content, $user)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$name = '';
		if (is_a($user, 'user')) {
			$name = $user->attr('first_name').' '.$user->attr('last_name');
		} else if (is_a($user, 'bulkmail_user')) {
			$name = $user->attr('username');
		}
		$keyword_replacements = Array(
									'bmail_assetid'		=> $user->id,
									'bmail_name'		=> $name,
									'bmail_email'		=> $user->attr('email'),
								);
		$content = replace_keywords($content, $keyword_replacements);

		return $content;

	}//end replaceContentKeyword()


	/**
	* Generates a standard preview email
	*
	* @param object	&$asset	the asset to which we belong
	*
	* @return string
	* @access public
	*/
	function generatePreviewEmail($asset)
	{
		$content_details = $asset->attr('content_details');
		$content_details['content_page'] = $asset->attr('content_assetid');
		$bmail_content = $this->generateBmailContent($asset, $content_details);
		$preview_email = "This is a preview of Bulkmail Job (assetid:".$asset->id.")<br />";
		$preview_email .= "<h1>raw html content</h1><br />".$bmail_content;

		return $preview_email;

	}//end generatePreviewEmail()


	/**
	* Sends the bulkmail message using given PEAR mail driver params
	*
	* @param mixed string|array	$recipient	an array or a string with comma separated recipients.
	*
	* @return bool
	* @access public
	*/
	function sendPreviewMail($asset, $recipient)
	{
		if(is_null($asset->getPostOffice()) ){
			// unable to create mail object
			trigger_localised_error('BML0005', E_USER_NOTICE, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'), $asset->name, $asset->id);
			exit;
		}

		// Create the mail object using server_details as parameter
		$server_details = $asset->getServerDetails();
		$driver = array_get_index($server_details, 'driver', '');
		$mail_object =& Mail::factory($driver, $server_details);
		// integrate PEAR error object's message with matrix's error reporting system
		if(PEAR::isError($mail_object)) {
			// unable to create mail object
			trigger_localised_error('BML0001', E_USER_WARNING, $mail_object->getMessage());
			exit;
		}

		$header_details = $asset->getHeaderDetails();
		$content = $this->generatePreviewEmail($asset);
		$status = $mail_object->send($recipient, $header_details, $content);
		if (PEAR::isError($status)) {
			// unable to send bulkmail
			trigger_localised_error('BML0002', E_USER_WARNING, $status->getMessage());
			exit;
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
		$who = implode("&nbsp; ", $recipient).'<br /> on ';
		$when = date('r');

		return $msg."&nbsp; ".$who.$when;
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

	}//end getStatus()


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
			if($status == SQ_STATUS_LIVE) {
				return $job_assetid;
			}
		}

		return false;

	}//end hasLiveChild()


}//end class
?>