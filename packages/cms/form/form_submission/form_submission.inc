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
* $Id: form_submission.inc,v 1.50 2013/08/15 05:48:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Submission
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.50 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Submission extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Submission($assetid=0)
	{
		$this->_ser_attrs = TRUE;


		return $this->Asset($assetid);

	}//end constructor


	/**
	* Return TRUE if this link can be cloned
	*
	* @return boolean
	* @access public
	*/
	public function canCloneLink()
	{
		return FALSE;

	}//end isDeletableLink()


	/**
	* returns the answer of an question
	*
	* @param int	$id	the id of the question
	*
	* @return mixed boolean|string
	* @access public
	*/
	function getAnswer($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['answer'])) {
			return $a['answers'][$id]['answer'];
		}
		return FALSE;

	}//end getAnswer()


	/**
	* returns the Extra Data of an question - returns an empty array if not exist
	*
	* @param int	$id	the id of the question
	*
	* @return Array
	* @access public
	*/
	function getExtraData($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['extra_data'])) {
			return $a['answers'][$id]['extra_data'];
		}
		return Array();

	}//end getExtraData()


	/**
	* returns the name of an question
	*
	* @param int	$id	the id of the question
	*
	* @return string
	* @access public
	*/
	function getName($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['name'])) {
			return $a['answers'][$id]['name'];
		}
		return FALSE;

	}//end getName()


	/**
	* sets the answer of a question in this submission object
	*
	* @param int	$id		the id of the question
	* @param mixed	$value	the value of the question
	*
	* @return boolean
	* @access public
	*/
	function setAnswer($id, $value)
	{
		$a = $this->attr('attributes');
		if (empty($a['answers'])) $a['answers'] = Array();

		$a['answers'][$id]['answer'] = $value;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setAnswer()


	/**
	* sets the Extra Data for a question's response
	*
	* @param int	$id		the id of the question
	* @param Array	$data	the Extra Data of the question
	*
	* @return boolean
	* @access public
	*/
	function setExtraData($id, Array $data=NULL)
	{
		$a = $this->attr('attributes');
		if (empty($a['answers'])) $a['answers'] = Array();

		$a['answers'][$id]['extra_data'] = $data;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setAnswer()


	/**
	* set a form error into the error store
	*
	* @param string	$error	the error to set
	*
	* @return boolean
	* @access public
	*/
	function setError($error)
	{
		$a = $this->attr('attributes');
		if (!isset($a['is_error']) || !$a['is_error']) {
			$a['is_error'] = TRUE;
		}
		if (!isset($a['errors'])) $a['errors'] = Array();
		$a['errors'][] = $error;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}
		return TRUE;

	}//end setError()


	/**
	* get the form errors from the store
	*
	* @return array
	* @access public
	*/
	function getErrors()
	{
		$a = $this->attr('attributes');
		return array_get_index($a, 'errors', Array());

	}//end getErrors()


	/**
	* flush the errors in the store
	*
	* @return boolean
	* @access public
	*/
	function flushErrors()
	{
		$a = $this->attr('attributes');
		$a['is_error'] = FALSE;
		$a['errors'] = Array();

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end flushErrors()


	/**
	* returns true if there is a form error
	*
	* @return boolean
	* @access public
	*/
	function isError()
	{
		$a = $this->attr('attributes');
		return (isset($a['is_error']) && ($a['is_error'])) ? TRUE : FALSE;

	}//end isError()


	/**
	* returns the answers for question in the form:
	*
	* @return array
	* @access public
	*/
	function getAnswers()
	{
		$a = $this->attr('attributes');
		return array_get_index($a, 'answers', Array());

	}//end getAnswers()


	/**
	* sets a summary about a question used to print a complete summary of the answers of this form
	*
	* @param int	$question_id	the id of the question
	* @param string	$name			the name of the question
	* @param string	$value			the value of the question
	*
	* @return boolean
	* @access public
	*/
	function setSummary($question_id, $name, $value)
	{
		$a = $this->attr('attributes');

		if (empty($a['summary'])) $a['summary'] = Array();

		$a['summary'][$question_id]['answer']  = $value;
		$a['summary'][$question_id]['name']    = $name;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setSummary()


	/**
	* returns an Array with summary information
	*
	* @param int	$q_id	Question id
	* @param string	$value	Value
	*
	* @return array
	* @access public
	*/
	function getSummary($q_id=NULL, $value='answer')
	{
		$a = $this->attr('attributes');
		if (!$a || !isset($a['summary'])) return FALSE;

		if (is_null($q_id)) {
			return $a['summary'];
		} else {
			return $a['summary'][$q_id][$value];
		}

	}//end getSummary()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	* @param boolean	$log_message	if FALSE attrubutes change won't be logged in system log
	*
	* @return boolean
	* @access public
	* @see asset::setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE, $log_message=TRUE)
	{

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$val = parent::saveAttributes($dont_run_updated, $log_message);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return $val;

	}//end saveAttributes()


	/**
	* Increments the version number of the asset based on the part of the version passed in
	*
	* This will override incrementVersion() so the parents of form submissions
	* never get version increases linked to stuff that form submissions do. This
	* is for speed purposes, and generally cuts out half the number of queries
	* required to create a new submission
	*
	* @param string		$number			the part of the version to update (major|minor|micro)
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access public
	*/
	function incrementVersion($number='micro', $update_parents=TRUE)
	{
		return parent::incrementVersion($number, FALSE);

	}//end incrementVersion()


	/**
	* Called to be printed before the design is finished
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (isset($_GET['q']) && $_GET['q'] && (SQ_IN_BACKEND || SQ_IN_LIMBO) && $this->readAccess()) {
			// Download a file upload, but only if we are on the backend somewhere
			$question_assetid = $_GET['q'];
			$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_assetid, '', TRUE);

			if ($question_asset && ($question_asset instanceof Form_Question_Type_File_Upload)) {
				$extra_data = $this->getExtraData($question_assetid);
				if (!empty($extra_data['filesystem_path'])) {
					$filepath = $extra_data['filesystem_path'];
					if(strpos($filepath, '/') !== 0) $filepath = SQ_SYSTEM_ROOT.'/'.$filepath;
					if (is_file($filepath)) {
						require SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
						$type = array_get_index($standards_lists_mime_types, get_file_type($filepath), 'application/octet-stream');

						header('Content-type: '.$type);
						header('Content-disposition: attachment; filename="'.(basename($filepath)).'"');
						header('Content-length: '.filesize($filepath));
						echo file_get_contents($filepath);
					}
				}
			}

			exit(0);
		} else {
			parent::printFrontend();
		}

	}//end printFrontend()
	

	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$summary = $this->attr('attributes');
		$summary = isset($summary['summary']) ? $summary['summary'] : Array();
		?>
		<table>
			<thead>
				<tr>
					<th>Question</th>
					<th>Answer</th>
				</tr>
			</thead>
			<tbody>
			<?php
			foreach ($summary as $item) {
				?>
				<tr>
					<td><?php echo $item['name']; ?></td>
					<td><?php echo $item['answer']; ?></td>
				</tr>
				<?php
			}
			?>
			</tbody>
		</table>
		<?php

	}//end printBody()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$matches = Array();
		$original_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);

		if (preg_match('/question\_name\_q([0-9]+)/', $keyword, $matches)) {
			$attrs = $this->attr('attributes');			
			if (!isset($attrs['summary'])) return '';

			foreach ($attrs['summary'] as $name => $value) {
				$q_ids = explode(':q', $name);
				if ($q_ids[1] == $matches[1]) {
					apply_keyword_modifiers($value['name'], $modifiers, Array('assetid' => $this->id));
					return $value['name'];
				}
			}
			return '';
		}

		if (preg_match('/question\_name\_(([0-9]+)\_q([0-9]+))/', $keyword, $matches)) {
			$attrs = $this->attr('attributes');			
			if (!isset($attrs['summary'])) return '';

			$elt = array_get_index($attrs['summary'], $matches[2].':q'.$matches[3], Array('name' => ''));
			apply_keyword_modifiers($elt['name'], $modifiers, Array('assetid' => $this->id));
			return $elt['name'];
		}

		if (preg_match('/question\_answer\_q([0-9]+)/', $keyword, $matches)) {
			$attrs = $this->attr('attributes');			
			if (!isset($attrs['summary'])) return '';

			foreach ($attrs['summary'] as $name => $value) {
				$q_ids = explode(':q', $name);
				if ($q_ids[1] == $matches[1]) {
					apply_keyword_modifiers($value['answer'], $modifiers, Array('assetid' => $this->id));
					return $value['answer'];
				}
			}
			return '';
		}

		if (preg_match('/question\_answer\_(([0-9]+)\_q([0-9]+))/', $keyword, $matches)) {
			$attrs = $this->attr('attributes');
			if (!isset($attrs['summary'])) return '';

			$elt = array_get_index($attrs['summary'], $matches[2].':q'.$matches[3], Array('answer' => ''));
			apply_keyword_modifiers($elt['answer'], $modifiers, Array('assetid' => $this->id));
			return $elt['answer'];
		}

		if (preg_match('/question\_answer\_key\_q([0-9]+)/', $keyword, $matches)) {
			$attrs = $this->attr('attributes');			
			foreach ($attrs['answers'] as $name => $value) {
				$q_ids = explode(':q', $name);
				if ($q_ids[1] == $matches[1]) {
					if (!is_scalar($value['answer'])) {
						$value = array_get_index($attrs['summary'], $name, Array('answer' => ''));
					}
					apply_keyword_modifiers($value['answer'], $modifiers, Array('assetid' => $this->id));
					return $value['answer'];
				}
			}
			return '';
		}

		//do keyword replacement for question types that have both keys and values in the answers such as select or country question types
		if (preg_match('/question\_answer\_key\_(([0-9]+)\_q([0-9]+))/', $keyword, $matches)) {
			$attrs = $this->attr('attributes');
			$elt = array_get_index($attrs['answers'], $matches[2].':q'.$matches[3], Array('answer' => ''));
			//return the summary if the answers key is not a scalar value such as tickbox list question type
			if (!is_scalar($elt['answer'])) {
				$elt = array_get_index($attrs['summary'], $matches[2].':q'.$matches[3], Array('answer' => ''));
			}
			apply_keyword_modifiers($elt['answer'], $modifiers, Array('assetid' => $this->id));
			return $elt['answer'];
		}

		// if we are falling back on parent to get the keyword replacement make
		// sure we pass the whole keyword i.e. including the modifier(s) if any
		return parent::getKeywordReplacement($original_keyword);

	}//end getKeywordReplacement()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();
		$res['question_name_X_qY'] = 'The name of the question with ID X:qY';
		$res['question_answer_X_qY'] = 'The answer to the question with ID X:qY';
		$res['question_answer_key_X_qY'] = 'The answer key to the question with ID X:qY';
		$res['submission_ip_address'] = 'IP address that created this submission';
		return $res;

	}//end getAvailableKeywords()


	/**
	* returns the IP of a submission
	*
	* @return mixed boolean|string
	* @access public
	*/
	function getIP()
	{
		$a = $this->attr('attributes');
		if (isset($a['misc']['ip'])) return $a['misc']['ip'];
		return FALSE;

	}//end getIP()


	/**
	* sets the IP of a submission object
	*
	* @param string	$ip	the ip of the user who submitted
	*
	* @return boolean
	* @access public
	*/
	function setIP($ip)
	{
		$a = $this->attr('attributes');
		if (empty($a['mics'])) $a['misc'] = Array();

		$a['misc']['ip'] = $ip;

		if (!$this->setAttrValue('attributes', $a)) {
			return FALSE;
		}

		return TRUE;

	}//end setIP()


	/**
	* Get the IP of the submission
	*
	* @return string
	* @access public
	*/
	function getSubmissionIpAddressKeywordReplacement()
	{
		return $this->getIP();

	}//end getSubmissionIpAddressKeywordReplacement()


	/**
	* Sets a random unqiue passcode to the submission asset
	* Relevant to Public User submissions only
	*
	* @return string|boolean
	* @access public
	*/
	function setSubmissionPasscode()
	{
		$existing_passcode = $this->attr('passcode');
		//This is to be set only ever once
		if (!empty($existing_passcode)) {
			return $existing_passcode;
		}

		$unique_code = str_pad(rand(0,999), 3, '0', STR_PAD_LEFT).uniqid();
		if (!$this->setAttrValue('passcode', $unique_code)) {
			return FALSE;
		}

		return $unique_code;

	}//end setSubmissionPasscode()


	/**
	* Returns the passcode set to submission asset
	*
	* @return string|boolean
	* @access public
	*/
	function getSubmissionPasscode()
	{
		return $this->attr('passcode');

	}//end getSubmissionPasscode()


	/**
	* If the submission asset is restricted to admin users only
	* as per form asser settings to which the submission belongs
	*
	* @return boolean
	* @access public
	*/
	public function permissionRestricted()
	{
		if (isset($this->_tmp['restricted_submission_access'])) {
			return $this->_tmp['restricted_submission_access'];
		}

		// Get the submission's form assetid
		$sql = "SELECT
					a.assetid
				FROM
					sq_ast a, sq_ast_lnk l
				WHERE
					a.assetid = l.majorid AND
					l.is_exclusive ='1' AND
					l.value='submissions_folder' AND
					type_code = 'form_email' AND
					l.minorid IN (
						SELECT majorid FROM sq_ast_lnk WHERE minorid = :submission_id AND link_type = :link_type
					)
				";
		$prepare = MatrixDAL::preparePdoQuery($sql);
		try {
			MatrixDAL::bindValueToPdo($prepare, 'submission_id', $this->id);
			MatrixDAL::bindValueToPdo($prepare, 'link_type', SQ_LINK_TYPE_3);
			$form_assetid = MatrixDAL::executePdoOne($prepare);
		} catch (Exception $e) {
			throw new Exception('Unable to get submission\'s form asset id information due to database error: '.$e->getMessage());
		}

		$restricted = FALSE;
		if ($form_assetid) {
			$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_assetid);
			if (!is_null($form)) {
				$restricted = $form->attr('restricted_submission_access');
			}
		}
		$this->_tmp['restricted_submission_access'] = $restricted;

		return $restricted;

	}//end _adminRestricted()


	/**
	* Returns TRUE if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	public function readAccess(Array $assetids=Array())
	{
		if ($this->permissionRestricted()) {
			// Admin permission is required to read this submission
			return $this->adminAccess('', $assetids);
		}

		return parent::readAccess();

	}//end readAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has write access to this asset
	*
	* The extra parameter, $only_workflow, is currently only be used in
	* determining read access for an asset before it goes live.
	*
	* @param string		$lock_type		the type of write access required - if blank returns a plain check against
	*									permissions set for this asset, otherwise returns effective access based
	*									on locking and status
	* @param array		$assetids		An array of user_group and/or user ids to check for write access
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	If TRUE (default), users with write access normally will be denied write access
	*									if outside the current step of the workflow. If FALSE, both those in the workflow
	*									and those with normal write access will be granted write access.
	*
	* @return boolean
	* @access public
	*/
	public function writeAccess($lock_type='', Array $assetids=Array(), $only_workflow=TRUE)
	{
		if ($this->permissionRestricted()) {
			// Admin permission is required to read this submission
			return $this->adminAccess($lock_type, $assetids);
		}

		return parent::writeAccess();

	}//end writeAccess()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = Array(
						SQ_LINK_NOTICE	=> Array(
											'file'	=> Array(
														'card'		=> 'M',
														'exclusive'	=> FALSE,
													   ),
										   ),
					  );
		return $page_links;

	}//end _getAllowedLinks()


}//end class
?>
