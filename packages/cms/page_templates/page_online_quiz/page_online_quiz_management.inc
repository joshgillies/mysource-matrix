<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_online_quiz_management.inc,v 1.8 2007/12/06 05:03:07 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Online_Quiz_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Page_Online_Quiz_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						// overrides for default options
						// set up "types" to have questions already selected on create
						'types'					=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(
																	'online_quiz_question'	=> 1,
																   ),
												   ),
						// remove 'a-z' and 'random' from the group-by dropdown
						'group_by'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'number',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'number'	=> 'Standard',
																							'grouped'	=> 'Custom Grouping',
																						   ),
																	   ),
												   ),
						'num_questions'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'The number of questions to draw from the question pool (all, if zero)',
												   ),
						'user_details_source'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'current_user',
													'description'	=> 'Where to source the user details from: either the currently logged in user, or a form',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'current_user'	=> 'Current User',
																							'form'			=> 'Form',
																						   ),
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'can_retake'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Whether or not the user can take the quiz again',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'after_submit_action'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'thank_you',
													'description'	=> 'What to do after a quiz has been finished (eg. display a "thank you" page)',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'thank_you'			=> 'Show "Thank You" page',
																							'return_to_quiz'	=> 'Return to quiz immediately',
																						   ),
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'results_email_format'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'email_format',
													'default'		=> Array(),
													'description'	=> 'The format of the email that will be sent out at the completion of the quiz',
												   ),
						'clientside_validation'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Whether or not to warn with user with a JS popup when submitting if some answers are empty or invalid',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'log_submissions'		=> Array(
													'added'			=> '0.3',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'When enabled, User Details and submitted Quiz results will be logged. These will then be available to export from the &quot;Submission Logs&quot; screen',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
					  );

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('UPGRADING ONLINE QUIZ TO v0.3');
			$quizids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_online_quiz', FALSE);
			foreach ($quizids as $quizid) {
				$quiz = $GLOBALS['SQ_SYSTEM']->am->getAsset($quizid);
				if (!$quiz->_createSubmissionsFolder()) {
					trigger_error('Error creating submissions folder for Online Quiz #'.$quizid, E_USER_WARNING);
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($quiz);
			}
			pre_echo(count($quizids).' ONLINE QUIZ ASSETS UPDATED');
		}

		return $res;

	}//end _upgrade()


}//end class

?>
