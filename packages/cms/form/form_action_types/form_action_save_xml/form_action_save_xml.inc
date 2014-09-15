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
* $Id: form_action_save_xml.inc,v 1.11 2013/05/16 10:04:56 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';

/**
* Form_Action_Save_Xml
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/


class Form_Action_Save_Xml extends Form_Action
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}

		// Paint something here!
		?>
		<table class="no-borders" style="width:auto">
			<tr>
				<td><?php echo translate('location');?></td>
				<td>
					<?php
					$location = array_get_index($xml_settings, 'location', '');
					text_box($prefix.'_location', $location, 50);
					// using relative path
					if(strpos($location, '/') !== 0 && !empty($location)) {
					    $location = SQ_SYSTEM_ROOT.'/'.$location;
					}
					if (empty($location) || !is_dir($location) || !is_writable($location)) {
						?><br />
						<span class="sq-backend-warning"><?php echo translate('location_warning'); ?></span>
						<?php
					}//end if
					?>
						<div class="sq-backend-smallprint"><?php echo translate('use_relative_path');?></div>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('xml_detail');?></td>
				<td>
					<?php
					$detail = array_get_index($xml_settings, 'detail', 'simple');
					$options = Array(
								'simple'	=> 'Simple',
								'complex'	=> 'Complex',
							   );
					combo_box($prefix.'_detail', $options, FALSE, $detail);
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('xml_key');?></td>
				<td>
					<?php
					$key = array_get_index($xml_settings, 'key', 'value');
					$options = Array(
								'key'	=> 'Keys',
								'value'	=> 'Values',
							   );
					combo_box($prefix.'_key', $options, FALSE, $key);
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('xml_answer_sort');?></td>
				<td>
					<?php
					$key = array_get_index($xml_settings, 'sort', '0');
					check_box($prefix.'_sort', 1, $key);
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	*
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}

		$xml_settings['location'] = array_get_index($_POST, $prefix.'_location', '');
		$xml_settings['detail'] = array_get_index($_POST, $prefix.'_detail', 'simple');
		$xml_settings['key'] = array_get_index($_POST, $prefix.'_key', 'value');
		$xml_settings['sort'] = array_get_index($_POST, $prefix.'_sort', '0');

		$settings['save_xml'] = $xml_settings;

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	*
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}
		self::_fillDefaultValues($xml_settings);

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Location:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($xml_settings['location'])) {
						$location = $xml_settings['location'];
						// using relative path
						if(!empty($location) && strpos($location, '/') !== 0 ) {
						    $location = SQ_SYSTEM_ROOT.'/'.$location;
						}
						echo ellipsisize($location, 512);
					} else {
						?><span class="sq-backend-warning">No Location specified.</span><?php
					} ?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	*/
	public static function execute(Form $form, $settings)
	{
		$xml_settings = $settings['save_xml'];

		// Double check
		$check = self::isValid($form, $settings);
		if (!$check) return FALSE;

		// Some base variables
		$location = $xml_settings['location'];
		if(!empty($location) && strpos($location, '/') !== 0 ) {
		    $location = SQ_SYSTEM_ROOT.'/'.$location;
		}
		$detail = ($xml_settings['detail'] == 'complex') ? TRUE : FALSE;
		$use_keys = ($xml_settings['key'] == 'key') ? TRUE : FALSE;
		$sort_ans = ($xml_settings['sort'] == '1') ? TRUE : FALSE;
		$questions = $form->getAllQuestions();
		$submission = $form->getSubmissionAsset();
		$complete = $submission->attr('complete');
		$attributes = $submission->attr('attributes');
		$answers = array_get_index($attributes, 'answers', Array());
		$file = $location.'/submission-'.$submission->id.'-'.date('YmdHis', $submission->created).'.xml';

		// Whether to sort by the sort settings in the form/form section
		// By default, questions are sorted by assetid
		if ($sort_ans) {
			$sections = $form->getAllSections();
			$sort_order = $form->attr('sort_order');
			if (!empty($sections)) {
			    foreach($sections as $section) {
			        $sort_order = array_merge($sort_order, $section->attr('sort_order'));
			    }//end foreach
			}//end if

			$sorted_ans = Array();
			foreach($sort_order as $ordered_qid) {
				if (isset($answers[$ordered_qid])) {
			        $sorted_ans[$ordered_qid] = $answers[$ordered_qid];
			    }//end if
			}//end foreach
			$answers = $sorted_ans;
			unset($sorted_ans);
		}

		// Start the submission XML
		if ($complete && !empty($answers)) {
			// Process the submission and save as XML
			$sxml = new SimpleXMLElement('<submission></submission>');
			$sxml->addAttribute('id', $submission->id);
			$sxml->addAttribute('form', $form->id);
			$sxml->addAttribute('time', date('YmdHis', $submission->created));
			// If detail selected, use the extra attributes
			if ($detail) {
				if (isset($attributes['misc']['ip'])) {
					$sxml->addAttribute('ip', $attributes['misc']['ip']);
				}//end if
				if (!empty($submission->updated_userid)) {
					$sxml->addAttribute('user', $submission->created_userid);
				}//end if
			}//end if

			// Process the answers
			foreach ($answers as $question_id => $answer) {
				// Get the question details
				if (isset($questions[$question_id])) {
					$type = str_replace('form_question_type_', '', $questions[$question_id]['question_type_code']);
					$question_name = $questions[$question_id]['attributes']['name'];
				} else {
					$type = '';
					$question_name = '';
				}//end if

				// Format the reponse
				$value = $answer['answer'];

				// Do not check for an empty value here, as types like an option list use '0' has a valid value
				if (!empty($type)) {
					switch ($type) {
						case 'tickbox_table':
							$repsonses = Array();
							// Clean up the table so values can be pulled out accurately
							if (isset($questions[$question_id])) {
								$ttable = @unserialize($questions[$question_id]['attributes']['question_contents']);
								$waste = array_shift($ttable);
								foreach ($ttable as $id => $options) {
									unset($ttable[$id]);
									$another_waste = array_shift($options);
									$ttable[$id] = $options;
								}//end foreach
							}//end if
							foreach ($value as $qid => $response) {
								foreach ($response as $ra => $stuff) {
									if (isset($ttable[$qid][$ra])) {
										$responses[] = htmlspecialchars($ttable[$qid][$ra]);
									} else {
										$responses[] = $ra;
									}//end if
								}//end foreach
							}//end foreach
							$value = $responses;
						break;
						case 'select':
						case 'tickbox_list':
						case 'option_list':
							if (is_array($value)) {
								if (isset($questions[$question_id]['attributes']['options']) && !$use_keys) {
									$list_options = @unserialize($questions[$question_id]['attributes']['options']);
									foreach ($value as $id => $name) {
										if (isset($list_options[$name])) {
											unset($value[$id]);
											$value[$id] = htmlspecialchars($list_options[$name]);
										}//end if
									}//end foreach
								}//end if
							} else {
								if (isset($questions[$question_id]['attributes']['options']) && !$use_keys) {
									$list_options = @unserialize($questions[$question_id]['attributes']['options']);
									if (isset($list_options[$value])) {
											$value = htmlspecialchars($list_options[$value]);
									}//end if
								}//end if
							}//end if
						break;
						case 'country':
							require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';
							global $standards_lists_countries;
							if (is_array($value)) {
								foreach ($value as $id => $name) {
									if (isset($standards_lists_countries[$name]) && !$use_keys) {
										unset($value[$id]);
										$value[$id] = htmlspecialchars($standards_lists_countries[$name]);
									}//end if
								}//end foreach
							} else {
								if (isset($standards_lists_countries[$value]) && !$use_keys) {
									$value = htmlspecialchars($standards_lists_countries[$value]);
								}//end if
							}//end if
						break;
						case 'file_upload':
							$filename = $value;
							$value = '';
							$assetid = 0;
							$fileid = 0;
							if (isset($answer['extra_data']['new_file_assetid']) && !empty($answer['extra_data']['new_file_assetid'])) {
								$assetid = $answer['extra_data']['new_file_assetid'];
							}//end if
							if (isset($answer['extra_data']['existing_file_assetid']) && !empty($answer['extra_data']['existing_file_assetid'])) {
								$assetid = $answer['extra_data']['existing_file_assetid'];
							}//end if

							if (!empty($assetid)) {
								$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
								if (!is_null($asset)) {
									$file_details = $asset->getExistingFile();
									$file_to_process = $file_details['path'];
									$filename = $file_details['filename'];
									if (file_exists($file_to_process)) {
										$fhandle = fopen($file_to_process, 'r');
										$file_content = fread($fhandle, filesize($file_to_process));
										fclose($fhandle);
										$value = chunk_split(base64_encode($file_content));
									}//end if
								}//end if

								$fileid = $assetid;
							}//end if
						break;
						case 'datetime':
							$timestamp = iso8601_ts($value);
							if ($timestamp !== FALSE) {
								$value = date('Ymd', $timestamp);
							}//end if
						break;
					}//end switch
				}//end if

				// Format the value ready for XML processing
				if (is_array($value)) {
					if (!empty($value)) {
						$value = implode('|', $value);
					} else {
						$value = '';
					}//end if
				}//end if

				// Compile the XML
				$response = $sxml->addChild('response', utf8_encode($value));
				if (!empty($question_name)) {
					$response->addAttribute('name', utf8_encode($question_name));
				}//end if
				if (!empty($type)) {
					$response->addAttribute('type', $type);
					if ($type == 'file_upload' && !empty($filename)) {
						$response->addAttribute('filename', $filename);
						if (isset($fileid) && !empty($fileid)) {
							$response->addAttribute('assetid', $fileid);
						}//end if
					}//end if
				}//end if
				if ($detail) {
					$response->addAttribute('id', $question_id);
				}//end if
			}//end foreach

			// Save the XML
			$xml = $sxml->asXML();

			$result = file_put_contents($file, $xml);
		} else {
			return FALSE;
		}//end if

		return $result;

	}//end execute()


	/**
	* isValid
	*
	*/
	public static function isValid(Form $form, $settings)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}
		self::_fillDefaultValues($xml_settings);

		// Check if valid and return FALSE
		if (!isset($xml_settings['location']) || empty($xml_settings['location'])) {
			return FALSE;
		}//end if

		$location = $xml_settings['location'];
		if(strpos($location, '/') !== 0) {
		    $location = SQ_SYSTEM_ROOT.'/'.$location;
		}

		if (!is_dir($location) || !is_writable($location)) {
			return FALSE;
		}//end if

		return TRUE;

	}//end isValid()


	/**
	* Fill any missing values from the xml format with defaults
	*
	* @param array	&$xml_format	The xml settings
	*
	* @return void
	*/
	private static function _fillDefaultValues(&$xml_format) {
		if (!isset($xml_format['location'])) {
			$xml_format['location'] = '';
		}
		if (!isset($xml_format['detail'])) {
			$xml_format['detail'] = 'simple';
		} else if (empty($xml_format['detail'])) {
			$xml_format['detail'] = 'simple';
		}//end if

	}//end _fillDefaultValues()


}//end class

?>
