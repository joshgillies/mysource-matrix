<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: report_links_edit_fns.inc,v 1.59 2013/08/27 05:36:33 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';

/**
* Report_Links_Edit_Fns
*
* Purpose
*	Generate a report of links
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.59 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Report_Links_Edit_Fns extends Report_Edit_Fns
{

	var $follow_redirect = FALSE;
	var $redirect_counter = 0;
	var $max_redirect = 10;
	

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = '0';

	}//end constructor


	/**
	* Generate this report until complete
	*
	* @param object	&$job		a reference to the array of information about
	* @param array	&$step_data	a reference to the array of information about
	*							the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$owner = $GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_links');

		$this->follow_redirect = $owner->attr('follow_link_redirect');
			
			
		if (!isset($job->_running_vars['results'])) {
			$list_content_types = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->getRootAssetid($owner), 'content_type', FALSE);
			$list_page_redirects = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->getRootAssetid($owner), 'page_redirect', FALSE);
			$job->_running_vars['todo'] = $list_content_types + $list_page_redirects;
			$job->_running_vars['done'] = Array();
			$job->_running_vars['results'] = Array();

			if (!is_dir($owner->data_path)) {
				if (!create_directory($owner->data_path)) {
					trigger_localised_error('CMS0005', E_USER_WARNING);
					return FALSE;
				}
			}
			$temp_file = fopen($owner->data_path.'/report.tmp', 'w');
			if ($temp_file === FALSE) {
				trigger_localised_error('RPRT0000', E_USER_WARNING);
				return FALSE;
			}
			fwrite($temp_file, '<?xml version="1.0" encoding="'.SQ_CONF_DEFAULT_CHARACTER_SET.'"?>'."\n");
			fwrite($temp_file, "<links>\n");
		} else {
			$temp_file = fopen($owner->data_path.'/report.tmp', 'a');
			if ($temp_file === FALSE) {
				trigger_localised_error('RPRT0000', E_USER_WARNING);
				return FALSE;
			}
		}
		if (!empty($job->_running_vars['todo'])) {
			$array_keys = array_keys($job->_running_vars['todo']);
			$assetid = array_shift($array_keys);
			$asset_type = $job->_running_vars['todo'][$assetid][0]['type_code'];
			unset($job->_running_vars['todo'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if ($asset_type == 'content_type_nest_content') {
				$step_data['message'] = translate('cms_report_links_skipping_nest_content', $assetid);
			}
			// skip thoese with unwanted status
			else if (isset($asset->status) && !in_array($asset->status,$owner->attr('statuses'))) {
				 $step_data['message'] = translate('cms_report_links_skipping_unwanted_status', $assetid);
			}
			else {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

				if (!is_null($asset)) {
					if (($asset instanceof Page_Redirect)) {
						// Look for Page_Redirect for links
						$step_data['message'] = translate('cms_report_links_generating');

						$content = '';
						$match = $asset->attr('redirect_url');
						$confirm_internal = $asset->getRedirectAsset();
						$validated_results = FALSE;

						if (!is_null($confirm_internal)) {
							// If no redirect_url, then an internal link
							if ($GLOBALS['SQ_SYSTEM']->am->assetExists($confirm_internal->id)) {
								// Because this is a special way of handling links with page redirect
								// We need to match the results as if the results have validated
								// Therefore the $match value is needed for the XML tp parse the correct asset
								$link_type = 'internal';
								$validated_results = Array('valid' => '1', 'description' => '');
								$match = './?a='.$confirm_internal->id;
							} else {
								$link_type = 'internal';
								$validated_results = Array('valid' => '1', 'description' => translate('cms_report_links_asset_not_exist', $confirm_internal->id));
								$match = './?a='.$confirm_internal->id;
							}
						} else {
							$results = '';
							preg_match('|^\./\?a=([0-9]+)#?([^$]*)?|',$match, $results);

							if (!empty($results)) {
								$link_type = 'internal';
								$validated_results = $this->validateInternalLink($results);
							} else {
								if (strpos(strtolower($match), 'mailto') === 0) {
									$link_type = 'mailto';
									$match = preg_replace('/mailto:/i','',$match);
									list($match) = explode('?',$match);
									$match = trim($match);
									$match = ltrim($match, '/');
									$validated_results = $this->validateEmailLink($match);
								} else {
									$link_type = 'external';
									$validated_results = $this->validateExternalLink($match);
								}
							}

						}

						if ($validated_results !== FALSE) {

							$link_info = Array (
											'type'	=> $link_type,
											'url'	=> $match,
										 );
							$content = $this->_addReportContent($content,$link_info,$validated_results);

							if (!empty($content)) {
								$content = "\t".'<asset assetid="'.$asset->id.'" name="'.htmlSpecialChars($asset->name).'" short_name="'.htmlSpecialChars($asset->short_name).'" type_code="'.$asset->type().'" status="'.$asset->status.'">'."\n".$content."\t".'</asset>'."\n";
								fwrite($temp_file, $content);
							}
						}

						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						unset($asset);
					} else {
						// Finally, scan the HTML for links
						$step_data['message'] = translate('cms_report_links_generating');

						$content = '';
						$wysiwyg_html = $asset->attr('html');

						$master_matches = Array();
						preg_match_all('%(href|src)=[\'"]([^\'\s"]*)%', $wysiwyg_html, $master_matches);
						$matches = $master_matches[2];
						foreach ($matches as $match) {
							// skip keywords
							$keywords = retrieve_keywords_replacements($match);
							if (count($keywords) == 1 && $match == '%'.$keywords[0].'%') {
								continue;
							}

							if (!$match) continue;
							$link_valid = '0';
							$link_description = '';
							$link_type = '';
							$results = Array();
							preg_match('|^\./\?a=([0-9]+)#?([^$]*)?|',$match, $results);


							$validated_results = FALSE;
							if (!empty($results)) {
								$link_type = 'internal';
								$validated_results = $this->validateInternalLink($results);
							} else {
								if (strpos(strtolower($match), 'mailto') === 0) {
									$link_type = 'mailto';
									$match = preg_replace('/mailto:/i','',$match);
									list($match) = explode('?',$match);
									$match = trim($match);
									$validated_results = $this->validateEmailLink($match);
								} else {
									$link_type = 'external';
									$validated_results = $this->validateExternalLink($match);
								}
							}

							if ($validated_results === FALSE) continue;

							$link_info = Array (
											'type'	=> $link_type,
											'url'	=> $match,
										 );
							$content = $this->_addReportContent($content,$link_info,$validated_results);
						}//end foreach

						if (!empty($content)) {
							$content = "\t".'<asset assetid="'.$asset->id.'" name="'.htmlSpecialChars($asset->name).'" short_name="'.htmlSpecialChars($asset->short_name).'" type_code="'.$asset->type().'" status="'.$asset->status.'">'."\n".$content."\t".'</asset>'."\n";
							fwrite($temp_file, $content);
						}

						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
						unset($asset);
					}//end else
				} else {
					$step_data['message'] = translate('cms_report_links_skipping_asset', $assetid);
					$job->_addError(translate('cms_report_links_asset_not_exist', $assetid));
				}

			}//end else nest content cell

			// add this assetid to the done array so we dont do it again
			$job->_running_vars['done'][] = $assetid;
		}//end if

		if (empty($job->_running_vars['todo'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
			$job->_running_vars['complete'] = TRUE;

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['userid']);
				// update the who/when attribute
				$owner->setAttrValue('generated_date', $job->_running_vars['generated_date']);
				$owner->setAttrValue('generated_user', $user->name.' (#'.$user->id.')');
				// reset to empty string, so that the 'Report Broken Link Now' button can be printed/enabled
				$owner->setAttrValue('report_info', '');
				$owner->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			// move the temp file to become the real report XML file
			fwrite($temp_file, '</links>');
			if (!copy($owner->data_path.'/report.tmp', $owner->data_path.'/report.xml')) {
				trigger_localised_error('RPRT0001', E_USER_WARNING);
				return FALSE;
			}
			if (!unlink($owner->data_path.'/report.tmp')) {
				trigger_localised_error('RPRT0002', E_USER_WARNING);
			}
			// send broken link notification, if enabled
			if ($owner->attr('report_broken_links')) {
				$owner->reportBrokenLink();
			}

		} else {
			$total = count($job->_running_vars['todo']) + count($job->_running_vars['done']);
			$step_data['percent_done'] = (count($job->_running_vars['done']) / $total) * 100;
			$step_data['complete']	   = FALSE;
		}

		fclose($temp_file);
		return TRUE;

	}//end generateReport()


	/**
	* Add Report Content
	*
	* @param string $content	 The content field (in case something is added before this
	* @param array  $link_info  The link type and url
	* @param array  $results	 The validated results in the format<br />
	*
	* @return string
	* @access private
	*/
	function _addReportContent($content,$link_info,$results=Array('valid'=>'0','description'=>''))
	{
		$content .= "\t\t".'<link type="'.$link_info['type'].'" valid="'.$results['valid'].'">'."\n";
		$content .= "\t\t\t".'<html><![CDATA['.$link_info['url'].']]></html>'."\n";
		$content .= "\t\t\t".'<description><![CDATA['.$results['description'].']]></description>'."\n";
		$content .= "\t\t".'</link>'."\n";

		return $content;

	}//end _addReportContent()


	/**
	* Check the validity of an internal link
	*
	* @param array	$link	the internal link in the format:
	*						Array
	*						(
	*						[0] => ./?a=123#anchor_name
	*						[1] => 142
	*						[2] => anchor_name
	*						)
	*
	* @return array
	* @access public
	*/
	function validateInternalLink($link)
	{
		$asset = NULL;
		$assetid = $link[1];
		if ($assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
		}

		if (is_null($asset)) {
			// the asset was invalid
			$description = translate('cms_report_links_point_invalid_assetid');
			$valid = '0';
		} else if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid, TRUE)) {
			// the asset is in the trash
			$description = translate('cms_report_links_point_to_trash');
			$valid = '0';
		} else if (!empty($link[2])) {
			$contents = Array();
			$editable_contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($assetid);
			if (is_array($editable_contents)) {
				$contents = $editable_contents;
			}

			$anchorExists = FALSE;

			// check if we are looking for an interface for this asset
			if (preg_match('/\/_admin|\/_edit/', $link[2], $matches)) {
				$anchorExists = TRUE;
			} else if (preg_match('/^\?(.+=.*)*/', $link[2], $matches)) {
				$anchorExists = TRUE;
			} else {
			// otherwise lets traul the contents for a matching anchor

				foreach ($contents as $content) {
					if (preg_match('/<a\s+(name|id)="#?'.$link[2].'"(\s+(name|id)="#?'.$link[2].'")?(\s+href="[^"]+")?>/', $content, $matches)) {
						$anchorExists = TRUE;
						break;
					}
				}
			}

			if (!$anchorExists) {
				$valid = '0';
				$description = translate('cms_report_links_anchor_not_exist', $link[2]);
			} else {
				$valid = '1';
				$description = '';
			}
		} else {
			$valid = '1';
			$description = '';
		}
		if (!is_null($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		}

		return Array('valid' => $valid, 'description' => $description);

	}//end validateInternalLink()


	/**
	* Check the validity of an external link
	*
	* @param string	$url	the url being validated
	*
	* @return array
	* @access public
	*/
	function validateExternalLink($url)
	{
		// ignore bookmarks
		if (substr($url, 0, 1) == '#') return FALSE;

		// ignore potential simple edit query string element
		// e.g. ./?SQ_ACTION=logout or ?asset_ei_screen=metadata
		if (strcmp(substr($url, 0, 3), './?') == 0 || substr($url, 0, 1) == '?') {
			return Array('valid' => 1, 'description' => '');
		}

		$validated_results = Array(
								'valid'			=> '0',
								'description'	=> '',
							 );

		$url_parts = @parse_url($url);

		if (empty($url_parts['scheme'])) {
			$validated_results['description'] = translate('cms_report_links_missing_protocol');
			$validated_results['valid'] = '0';
		} else if (empty($url_parts['host'])) {
			$validated_results['description'] = translate('cms_report_links_missing_hostname');
			$validated_results['valid'] = '0';
		} else {
			if (empty($url_parts['path'])) {
				$url_parts['path'] = '/';
			}
			if (empty($url_parts['port']) && !empty($url_parts['scheme']) && $url_parts['scheme'] == 'https') {
				$url_parts['port'] = '443';
			} else {
				$url_parts['port'] = '80';
			}

			$url_path = $url_parts['path'];
			if (!empty($url_parts['query'])) {
				$url_path .= '?'.$url_parts['query'];
			}

			require_once(SQ_INCLUDE_PATH.'/general_occasional.inc');
			$socket = openSocketConnection($url_parts, $errno, $errstr, TRUE, 15);
			if (!SQ_PA_ENABLED || !$socket) {
				// if proxy authentication is not enabled, or fopen fails, do the old trick
				$socket = @fsockopen($url_parts['host'], $url_parts['port'], $errno, $errstr, 15);
			}
			if ($socket) {
				fwrite ($socket, 'GET '.$url_path." HTTP/1.0\r\nHost: ".$url_parts['host']."\r\n\r\n");
			}
			if (!$socket) {
				$validated_results['description'] = translate('cms_report_links_invalid_url');
				$validated_results['valid'] = '0';
			} else {
				stream_set_timeout($socket, 5);
				$response = fgets($socket);
				$headers = fgets($socket);
				$response = trim($response);
				$headers = trim($headers);
				$finished = (empty($headers)) ? TRUE : FALSE;
				while (!$finished) {
					$next_header = fgets($socket);
					$next_header = trim($next_header);
					if (empty($next_header)) {
						$finished = TRUE;
					} else {
						$headers .= $next_header."\r\n";
					}//end if
				}//end while

				$info = stream_get_meta_data($socket);
				$validated_results = $this->_checkResponse($headers, $response, $info, $url_parts);

				fclose($socket);
			}//end else

		}//end else

		return $validated_results;

	}//end validateExternalLink()


	/**
	* Check the response of the external link
	*
	* @param	string	$headers	the captured headers
	* @param	string	$response	the captured response
	* @param	array	$info		the captured stream info
	* @param	array	$url_info	the URL info belonging to the response
	*
	* @return array
	* @access private
	*/
	function _checkResponse($headers, $response, $info, $url_info)
	{
		if (preg_match('|30[127] |', $response)) {
		    if($this->follow_redirect && $this->redirect_counter < $this->max_redirect) {
				preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
				$location = array_get_index($matches, 1, '');
				if ($location) {
					// if the "location" is not absolute URL
					if (!preg_match('|^https?://|', $location)) {
						$location = $url_info['scheme'].'://'.$url_info['host'].'/'.ltrim($location, '/');
					}
				    // follow redirect link but limit recursive loop
				    $this->redirect_counter = $this->redirect_counter + 1;
				    $result = $this->validateExternalLink($location);
			    	$this->redirect_counter  = 0;
				    return $result;
				}
		    }
		}

		if (preg_match('|200 ok|i', $response)) {
			$description = '';
			$valid = '1';
		} else if (preg_match('|302 found|i', $response)) {
			$description = '';
			$valid = '1';
		} else if (preg_match('|304 not modified|i', $response)) {
			$description = '';
			$valid = '1';
		} else if (preg_match('|404 not found|i', $response)) {
			$description = translate('cms_report_links_404');
			$valid = '0';
		} else if (preg_match('|407 proxy authentication required|i', $response)) {
			$description = translate('cms_report_links_407');
			$valid = '0';
		} else if (preg_match('|301 moved|i', $response)) {
			$valid = '1';
			preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
			if (isset($matches[1])) {
				$description = translate('cms_report_links_301_with_new_link', $matches[1]);
			} else {
				$description = translate('cms_report_links_301');
			}
		} else if (preg_match('|302 moved|i', $response)) {
			// should this
			$valid = '1';
			preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
			if (isset($matches[1])) {
				$description = translate('cms_report_links_302_with_new_link', $matches[1]);
			} else {
				$description = translate('cms_report_links_302');
			}
		// IIS Specific error codes
		} else if (preg_match('|202 accepted|i', $response)) {
			$description = '';
			$valid = '1';
		} else if (preg_match('|301 moved permanently|i', $response)) {
			$valid = '1';
			preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
			if (isset($matches[1])) {
				$description = translate('cms_report_links_301_with_new_link', $matches[1]);
			} else {
				$description = translate('cms_report_links_301');
			}
		} else if (preg_match('|301 permanent redirect|i', $response)) {
			$valid = '1';
			preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
			if (isset($matches[1])) {
				$description = translate('cms_report_links_301_with_new_link', $matches[1]);
			} else {
				$description = translate('cms_report_links_301');
			}
		} else if (preg_match('|302 object moved|i', $response)) {
			// should this
			$valid = '1';
			preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
			if (isset($matches[1])) {
				$description = translate('cms_report_links_302_with_new_link', $matches[1]);
			} else {
				$description = translate('cms_report_links_302');
			}
		} else if (preg_match('|307 temporary redirect|i', $response)) {
			$valid = '1';
			preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
			if (isset($matches[1])) {
				$description = translate('cms_report_links_307_with_new_link', $matches[1]);
			} else {
				$description = translate('cms_report_links_307');
			}
		} else if (preg_match('|400 bad request|i', $response)) {
			$description = translate('cms_report_links_400');
			$valid = '0';
		} else if (preg_match('|401 access denied|i', $response)) {
			$description = translate('cms_report_links_401');
			$valid = '0';
		} else if (preg_match('|401 unauthorized|i', $response)) {
			$description = translate('cms_report_links_401');
			$valid = '0';
		} else if (preg_match('|403 forbidden|i', $response)) {
			$description = translate('cms_report_links_403');
			$valid = '0';
		} else if (preg_match('|403 forbidden|i', $response)) {
			$description = translate('cms_report_links_403');
			$valid = '0';
		} else if (preg_match('|404 page not available|i', $response)) {
			$description = translate('cms_report_links_404');
			$valid = '0';
		} else if (preg_match('|406 client browser does not|i', $response)) {
			$description = translate('cms_report_links_406');
			$valid = '0';
		} else if (preg_match('|408 request timed out|i', $response)) {
			$description = translate('cms_report_links_408');
			$valid = '0';
		} else if (preg_match('|412 precondition failed|i', $response)) {
			$description = translate('cms_report_links_412');
			$valid = '0';
		} else if ($info['timed_out']) {
			$description = translate('cms_report_links_server_timeout');
			$valid = '0';
		} else {
			//remove non ascii chars from response
			$response = preg_replace('/[^(\x20-\x7F)]*/','',$response);
			$description = translate('cms_report_links_unknown_response', $response);
			$valid = '0';
		}

		// Format the results
		$validated_results['valid'] = $valid;
		$validated_results['description'] = $description;

		// Return the results
		return $validated_results;

	}//end _checkResponse()


	/**
	* Check the validity of an mailto
	*
	* @param string	$mailto	the mailto being validated
	*
	* @return array
	* @access public
	*/
	function validateEmailLink($mailto)
	{
		require_once SQ_FUDGE_PATH.'/general/www.inc';
		if (!valid_email($mailto)) {
			$description = translate('cms_report_links_invalid_email');
			$valid = '0';
		} else {
			$description = '';
			$valid = '1';
		}

		return Array('valid' => $valid, 'description' => $description);

	}//end validateEmailLink()


	/**
	* Paint the interface for viewing a report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$asset, &$o, $prefix)
	{
		$reported_links = $asset->parseReport();
		$display_options = $asset->attr('display_options');

		// TODO: this translation has a 'note' prefix that is supposed to look like
		// 		a field header... could this be set up as a field?
		if (SQ_IN_BACKEND || SQ_IN_LIMBO) {
			echo translate('cms_report_links_current');
		}
		$o->closeSection();

		if ($display_options['internal_section'] == TRUE) {
			$o->openSection('Internal Links');
				$o->openField('');
					if (empty($reported_links['internal'])) {
						echo 'No internal links were found';
					} else {
						?>
						<table class="sq-backend-table" style="border: 1px solid #DDDDDD;" border="0">
							<tr>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('found_in') ?></th>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('linking_to') ?></th>
								<th class="sq-backend-table-header"><?php echo translate('comments'); ?></th>
							</tr>
						<?php
						foreach ($reported_links['internal'] as $assetid => $links) {
							foreach ($links as $link) {
								$from_info = $this->_getAssetReportInfo($assetid);
								$from_name = (strlen($from_info['name']) < 40) ? $from_info['name'] : ellipsisize($from_info['name'], 40);

								$to = NULL;
								if ($link['data']) {
									$to = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['data'], '', TRUE);
								}
								if (is_null($to)) {
									$to_type = '';
									$to_string = translate('unknown_asset_id', $link['data']);
									$to_bg = '';
									$to_status = '';
									$to_name = '';
								} else {
									$to_type = $to->type();
									$to_name = (strlen($to->name) < 40) ? $to->name : ellipsisize($to->name, 40);
									$to_string = '<span title="'.$to->name.'"> [#'.$to->id.'] '.$to_name.'</span>';
									$to_bg = get_asset_status_icon($to->status);
									$to_status = get_status_description($to->status);
								}

								?>
								<tr>
									<!-- ASSET STATUS -->
									<td class="sq-backend-table-cell" style="width: 120px; padding-top: 7px; padding-right: 0px;">
										<?php echo $from_info['bg']; ?> <?php echo $from_info['status']; ?>
									</td>

									<!-- ASSET ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($from_info['type'])) {
											?>
											<span title="<?php echo ucwords(str_replace('_', ' ', $from_info['type'])); ?>">
												<?php echo get_asset_type_icon($from_info['type']); ?>
											</span>
											<?php
										}
										?>
									</td>

									<!-- ASSET NAME -->
									<td class="sq-backend-table-cell" style="width: 260px; padding-right: 1px; padding-top:6px; text-align: left;">
										<a href="<?php echo $from_info['backend_url']; ?>" title="<?php echo translate('edit_this_asset'); ?>">[#<?php echo  $from_info['assetid']; ?>] <?php echo	$from_name ?></a>
									</td>

									<!-- LINKED STATUS -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-top: 7px; padding-right: 0px;">
											<?php echo $to_bg; ?><?php echo $to_status; ?>
									</td>

									<!-- LINKED ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($to_type)) {
											echo get_asset_type_icon($to_type);
										}
										?>
									</td>

									<!-- LINKED NAME -->
									<td class="sq-backend-table-cell" style="width: 190px; padding-right: 1px; padding-top:6px; text-align: left;">
										<?php echo $to_string; ?>
									</td>

									<!-- COMMENT -->
									<td class="sq-backend-table-cell">
										<?php
											if (!$link['valid']) {
												echo '<span style="color: #007EFA;"><b>';
											}
										echo $link['description'];
										if (!$link['valid']) echo '</b></span>';
										?>
									</td>
								</tr>
								<?php
							}//end foreach link
						}//end foreach asset links
						?>
						</table>
						<?php
					}//end else
				$o->closeField();
			$o->closeSection();
		}//end if internal section is true

		if ($display_options['external_section'] == TRUE) {
			$o->openSection(translate('external_links'));
				$o->openField('');
					if (empty($reported_links['external'])) {
						echo translate('cms_report_links_no_external_links');
					} else {
						$ast_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($reported_links['external']));
						?>
						<table class="sq-backend-table">
							<tr>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('found_in'); ?></th>
								<th class="sq-backend-table-header" style="width: 274px;"><?php echo translate('url'); ?></th>
								<th class="sq-backend-table-header"><?php echo translate('comments'); ?></th>
							</tr>
						<?php
						foreach ($reported_links['external'] as $assetid => $links) {
							foreach ($links as $link) {
								$from_info = $this->_getAssetReportInfo($assetid);
								$from_name = (strlen($from_info['name']) < 40) ? $from_info['name'] : ellipsisize($from_info['name'], 40);

								$name = (strlen($link['data']) < 50) ? $link['data'] : ellipsisize($link['data'], 50);
								$to_string = '<a href="'.$link['data'].'" target="_blank" title="'.$link['data'].'">'.$name.'</a>';
								?>
								<tr>
									<!-- ASSET STATUS -->
									<td class="sq-backend-table-cell" style="width: 120px; padding-top: 7px; padding-right: 0px;">
										<?php echo $from_info['bg']; ?> <?php echo $from_info['status']; ?>
									</td>

									<!-- ASSET ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($from_info['type'])) {
											?>
											<span title="<?php echo ucwords(str_replace('_', ' ', $from_info['type'])); ?>">
												<?php echo get_asset_type_icon($from_info['type']); ?>
											</span>
											<?php
										}
										?>
									</td>

									<!-- ASSET NAME -->
									<td class="sq-backend-table-cell" style="width: 190px; padding-right: 1px; padding-top:6px; text-align: left;">
										<a href="<?php echo $from_info['backend_url']; ?>" title="<?php echo translate('edit_this_asset') ?>">[#<?php echo (int) $from_info['assetid']; ?>] <?php echo $from_name; ?></a>
									</td>

									<!-- URL -->
									<td class="sq-backend-table-cell" style="width: 226px; padding-right: 1px; padding-top:6px; text-align:left;">
										<?php echo $to_string; ?>
									</td>

									<!-- COMMENTs -->
									<td class="sq-backend-table-cell">
										<?php
											if (!$link['valid']) {
												echo '<span style="color: #007EFA;"><b>';
											}
										echo $link['description'];
										if (!$link['valid']) echo '</b></span>';
										?>
									</td>
								</tr>
								<?php
							}//end foreach link
						}//end foreach asset links
						?>
						</table>
						<?php
					}//end else
				$o->closeField();
			$o->closeSection();
		}//end if external section is true

		if ($display_options['email_section'] == TRUE) {
			$o->openSection(translate('cms_report_links_email_links'));
				$o->openField('');
					if (empty($reported_links['mailto'])) {
						echo translate('cms_report_links_no_email_links');
					} else {
						?>
						<table class="sq-backend-table">
							<tr>
								<th colspan="3" class="sq-backend-table-header" style="width: 226px;"><?php echo translate('found_in'); ?></th>
								<th class="sq-backend-table-header" style="width: 274px;"><?php echo translate('email'); ?></th>
								<th class="sq-backend-table-header"><?php echo translate('comments'); ?></th>
							</tr>
						<?php
						foreach ($reported_links['mailto'] as $assetid => $links) {
							foreach ($links as $link) {
								$from_info = $this->_getAssetReportInfo($assetid);
								$to_string = $link['data'];
								$name = (strlen($from_info['name']) < 40) ? $from_info['name'] : ellipsisize($from_info['name'], 40);
								?>
								<tr>

									<!-- ASSET STATUS -->
									<td class="sq-backend-table-cell" style="width: 120px; padding-top: 7px; padding-right: 0px;">
										<?php echo $from_info['bg']; ?> <?php echo $from_info['status']; ?>
									</td>

									<!-- ASSET ICON -->
									<td class="sq-backend-table-cell" style="width: 16px; padding-right: 0px; padding-left: 0px;">
										<?php
										if (!empty($from_info['type'])) {
											?>
											<span title="<?php echo ucwords(str_replace('_', ' ', $from_info['type'])); ?>">
												<?php echo get_asset_type_icon($from_info['type']); ?>
											</span>
											<?php
										}
										?>
									</td>

									<!-- ASSET NAME -->
									<td class="sq-backend-table-cell" style="width: 190px; padding-right: 1px; padding-top:6px; text-align: left;">
										<a href="<?php echo $from_info['backend_url']; ?>" title="<?php echo translate('edit_this_asset'); ?>">[#<?php echo (int) $from_info['assetid']; ?>] <?php echo $name; ?></a>
									</td>

									<!-- ADDRESS -->
									<td class="sq-backend-table-cell" style="width: 226px; padding-right: 1px; padding-top:6px;">
										<?php echo $to_string; ?>
									</td>

									<!-- COMMENTS -->
									<td class="sq-backend-table-cell">
										<?php
											if (!$link['valid']) {
												echo '<span style="color: #007EFA;"><b>';
											}
										echo $link['description'];
										if (!$link['valid']) echo '</b></span>';
										?>
									</td>
								</tr>
								<?php
							}//end foreach link
						}//end foreach asset links
						?>
						</table>
						<?php
					}//end else
				$o->closeField();
			$o->closeSection();
		}//end if email_section is true

	}//end paintReport()


	/**
	* Get reporting information about an asset
	* Returns array('assetid', 'type', 'string', 'bg')
	*
	* @param string	$assetid	the id of the asset we are getting reporting info for
	*
	* @return array
	* @access private
	*/
	function _getAssetReportInfo($assetid)
	{
		$asset = NULL;
		if ($assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
		}
		if (is_null($asset)) {
			$asset_type = '';
			$asset_name = 'Unknown asset #'.$assetid;
			$asset_short_name = 'Unknown asset #'.$assetid;
			$asset_string = 'Unknown asset #'.$assetid;
			$asset_bg = 'FFFFFF';
			$asset_status = '';
			$asset_status_code = 0;
			$asset_url = '';
			$asset_preview_url = '';
		} else {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id);
			// It faults here if there is only one dependant
			$first_dep = array_pop($dependants);
			if (!empty($first_dep)) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($first_dep);
			} else {
				$parent = NULL;
			}
			if (!is_null($parent)) {
				$asset_type = $parent->type();
				$asset_name = $parent->name;
				$asset_short_name = $parent->short_name;
				$asset_bg = get_asset_status_icon($parent->status);
				$asset_status = get_status_description($parent->status);
				$asset_status_code = $parent->status;
				$assetid = $parent->id;
				$asset_url = $parent->getBackendHref();
				$asset_preview_url = $parent->getURL();
			} else {
				$asset_type = $asset->type();
				$asset_name = $asset->name;
				$asset_short_name = $asset->short_name;
				$asset_bg = get_asset_status_icon($asset->status);
				$asset_status = get_status_description($asset->status);
				$asset_status_code = $asset->status;
				$asset_url = $asset->getBackendHref();
				$asset_preview_url = $asset->getURL();
			}

			include_once SQ_FUDGE_PATH.'/general/general.inc';
			$asset_string = ellipsisize($asset_name, 40);

			$asset_string = '
				<a href="'.$asset_preview_url.'" target="_blank" title="'.$asset_name.'">'.$asset_string.'</a>';
		}//end else

		return Array(
				'assetid'		=> $assetid,
				'name'			=> $asset_name,
				'short_name'	=> $asset_short_name,
				'type'			=> $asset_type,
				'type_code'		=> $asset_type,
				'string'		=> $asset_string,
				'bg'			=> $asset_bg,
				'status'		=> $asset_status,
				'status_code'	=> $asset_status_code,
				'backend_url'	=> $asset_url,
			   );

	}//end _getAssetReportInfo()


	/**
	* Paints the display options available for this report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintDisplayOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$tick = sq_web_path('lib').'/web/images/tick.gif';
		$cross = sq_web_path('lib').'/web/images/cross.gif';
		$current_settings = $asset->attr('display_options');

		?>
		<script type="text/javascript">
			var prefix = '<?php echo $prefix; ?>';

			// Disables a sections child options, if the top option is deselected
			function change_section(section, obj) {
				var start = prefix + '_' + section + '_';
				document.getElementById(start + 'working').disabled = obj.checked ? false : true;
				document.getElementById(start + 'broken').disabled = obj.checked ? false : true;
			}
		</script>

		<table class="sq-backend-table" style="width: 300px;">
			<tr>
				<th class="sq-backend-table-header" colspan="2"><?php echo translate('cms_report_links_internal_links'); ?></th>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_section', 'Internal'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['internal_section'];
						if ($write_access) {
							check_box($prefix.'_internal_section', '1', $value, 'change_section("internal", this)');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_broken'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['internal_broken'];
						if ($write_access) {
							check_box($prefix.'_internal_broken', '1', $value, '', $current_settings['internal_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_working'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['internal_working'];
						if ($write_access) {
							check_box($prefix.'_internal_working', '1', $value, '', $current_settings['internal_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<th class="sq-backend-table-header" colspan="2"><?php echo translate('cms_report_links_external_links'); ?></th>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_section', 'External'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['external_section'];
						if ($write_access) {
							check_box($prefix.'_external_section', '1', $value, 'change_section("external", this)');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_broken'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['external_broken'];
						if ($write_access) {
							check_box($prefix.'_external_broken', '1', $value, '', $current_settings['external_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_working'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['external_working'];
						if ($write_access) {
							check_box($prefix.'_external_working', '1', $value, '', $current_settings['external_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<th class="sq-backend-table-header" colspan="2"><?php echo translate('cms_report_links_email_links'); ?></th>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_section', 'Email'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['email_section'];
						if ($write_access) {
							check_box($prefix.'_email_section', '1', $value, 'change_section("email", this)');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_broken'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['email_broken'];
						if ($write_access) {
							check_box($prefix.'_email_broken', '1', $value, '', $current_settings['email_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="padding-left: 30px;"><?php echo translate('cms_report_links_display_working'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
						$value = $current_settings['email_working'];
						if ($write_access) {
							check_box($prefix.'_email_working', '1', $value, '', $current_settings['email_section'] == FALSE ? 'disabled' : '');
						} else {
							?>
							<img src="<?php echo $value ? $tick : $cross; ?>" />
							<?php
						}
					?>
				</td>
			</tr>

		</table>

		<?php
		return $write_access;

	}//end paintDisplayOptions()


	/**
	* Paints the display options available for this report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDisplayOptions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return $write_access;
		$display_options = $asset->attr('display_options');

		// These conditions below ensure that the elements retain their values if they are disabled
		$display_options['internal_section'] = isset($_POST[$prefix.'_internal_section']);
		if ($display_options['internal_section']) {
			$display_options['internal_broken'] = isset($_POST[$prefix.'_internal_broken']);
			$display_options['internal_working'] = isset($_POST[$prefix.'_internal_working']);
		}

		$display_options['external_section'] = isset($_POST[$prefix.'_external_section']);
		if ($display_options['external_section']) {
			$display_options['external_broken'] = isset($_POST[$prefix.'_external_broken']);
			$display_options['external_working'] = isset($_POST[$prefix.'_external_working']);
		}

		$display_options['email_section'] = isset($_POST[$prefix.'_email_section']);
		if ($display_options['email_section']) {
			$display_options['email_broken'] = isset($_POST[$prefix.'_email_broken']);
			$display_options['email_working'] = isset($_POST[$prefix.'_email_working']);
		}

		$asset->setAttrValue('display_options', $display_options);
		return TRUE;

	}//end processDisplayOptions()

	/**
	* Paint the interface for selecting  statuses
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintStatusSelection(&$asset, &$o, $prefix)
    {
    	require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('statuses');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');

			$options = Array();
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}

			combo_box($prefix.'_statuses', $options, TRUE, $current);
		} else {
			if (empty($current)) {
				echo translate('cms_report_links_no_status_selected');
			} else {
				?><ul style="margin: 1px 15px;"><?php
				foreach ($current as $status) {
					echo '<li>';
					echo get_asset_status_icon($status);
					echo get_status_description($status);
					echo '</li>';
				}
				?></ul><?php
			}
		}
		return $write_access;
    }//end paintStatusSelection()


	/**
	* Processes the interface for selecting  statuses
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
    function processStatusSelection(&$asset, &$o, $prefix)
    {
    	// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$statuses = Array();
		if (isset($_POST[$prefix.'_statuses'])) {
			foreach ($_POST[$prefix.'_statuses'] as $status) {
				$statuses[] = $status;
			}
			$statuses = array_unique($statuses);
		}

		return $asset->setAttrValue('statuses', $statuses);
    }//end processStatusSelection()

	/**
	* Paints the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExport(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		echo hidden_field($prefix.'_export_switch', '0');

		$enabled = FALSE;
		$report_path = $asset->data_path.'/report.xml';
		if (file_exists($report_path)) $enabled = TRUE;
		$extras = '';

		// we're going to disable the button if no report is available
		if (!$enabled) $extras = 'disabled="disabled"';

		echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;', $extras);

		return TRUE;

	}//end paintExport()


	/**
	* Processes the export block
	*
	* This will generate a CSV file, and present a download box to the user
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExport(&$asset, &$o, $prefix)
	{
		if (!$_POST[$prefix.'_export_switch']) return TRUE;
		if (headers_sent()) {
			trigger_localised_error('CORE0273', E_USER_WARNING);
		} else {
			require_once SQ_FUDGE_PATH.'/csv/csv.inc';
			$csv = new CSV();
			$csv->setFilename(str_replace(' ', '_', $asset->name).'_report.csv');
			$csv->setDeliminator(',');

			$am = $GLOBALS['SQ_SYSTEM']->am;

			$logs = Array();

			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$report_path = $asset->data_path.'/report.xml';

			if (!is_file($report_path)) {
				echo 'This report has not been generated';
				return;
			}

			try {
				$root = simplexml_load_string(file_get_contents($report_path), 'SimpleXMLElement', LIBXML_NOCDATA);
			} catch (Exception $e) {
				 throw new Exception('Unable to parse report file "'.$report_path.'": '.$e->getMessage());
				return;
			}

			$headers = Array();
			$empty_headers = TRUE;
			$top_headers = TRUE;
			$display_options = $asset->attr('display_options');

			foreach ($root->asset as $asset_node) {
				$persistent_values = Array();
				// Have to initialise the headers
				$attribute_list = Array('assetid', 'name', 'short_name', 'type_code', 'status');
				if ($top_headers) {
					$headers = $attribute_list;
				}

				$top_headers = FALSE;

				foreach ($asset_node->link as $link) {
					$valid = (int) $link->attributes()->valid;
					$type = (string) $link->attributes()->type;
					$html = (string) $link->html;
					$description = (string) $link->description;

					$display_link = TRUE;
					switch ($type) {
						case 'internal':
							if (!$display_options['internal_section']) {
								$display_link = FALSE;
							} else {
								if ($valid) {
									if (!$display_options['internal_working']) {
										$display_link = FALSE;
									}
								} else {
									if (!$display_options['internal_broken']) {
										$display_link = FALSE;
									}
								}
							}
						break;
						case 'external':
							if (!$display_options['external_section']) {
								$display_link = FALSE;
							} else {
								if ($valid) {
									if (!$display_options['external_working']) {
										$display_link = FALSE;
									}
								} else {
									if (!$display_options['external_broken']) {
										$display_link = FALSE;
									}
								}
							}
						break;
						case 'mailto':
							if (!$display_options['email_section']) {
								$display_link = FALSE;
							} else {
								if ($valid) {
									if (!$display_options['email_working']) {
										$display_link = FALSE;
									}
								} else {
									if (!$display_options['email_broken']) {
										$display_link = FALSE;
									}
								}
							}
						break;
						default:
						break;
					}//end switch

					if ($display_link) {
						// We need this info only if we are displaying the link and only once for current asset in the outer loop
						if (empty($persistent_values)) {
							$parent_ast_info = $this->_getAssetReportInfo((int) $asset_node->attributes()->assetid);
							foreach ($attribute_list as $attr) {
								$value = '';
								// the xml data might not be the data we want to export
								if (isset($parent_ast_info[$attr])) {
									$value = array_get_index($parent_ast_info, $attr);
									if ($attr == 'status') {
										// use the numerical representation of assets status
										$value = array_get_index($parent_ast_info, 'status_code');
									}
								}//end if
								// Need to setup the persistent values for each of the nodes (like source asset, type etc)
								$persistent_values[] = $value;
							}//end foreach
						}//end if

						$row = $persistent_values;
						if ($empty_headers) $headers[] = 'type';
						$row[] = $type;
						if ($empty_headers) $headers[] = 'valid';
						$row[] = $valid;

						if ($empty_headers) $headers[] = 'html';
						$row[] = $html;
						if ($empty_headers) $headers[] = 'description';
						$row[] = $description;

						$logs[] = $row;
						$empty_headers = FALSE;
					}
				}
			}//end foreach

			foreach ($headers as $id => $value) {
				$headers[$id] = ucwords(str_replace('_', ' ', $value));
			}

			if (!empty($logs)) $csv->setFieldHeaders($headers);

			$csv->setValues($logs);

			// export with keyed fields
			$csv->export(TRUE);
			exit(0);
		}//end else

		return TRUE;

	}//end processExport()


	/**
	* Paints the future regeneration cron job section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFutureRegenerate(&$asset, &$o, $prefix)
	{
		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_regenerate_report');
		$rl_jobs = $cron_mgr->getJobs('cron_job_regenerate_report');
		$job_count = count($rl_jobs);
		for ($k = 0; $k < $job_count; $k++) {
			// only include jobs for this link report asset
			if ($rl_jobs[$k]->attr('asset') != $asset->id) {
				unset($rl_jobs[$k]);
			}
		}
		if (!empty($rl_jobs)) {

			$o->openField(translate('cms_report_links_future_regen_section'), 'new_line');

			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('when'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('status'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('run_as'); ?></td>
					<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php
					if ($asset->writeAccess('attributes')) {
						echo translate('delete_question');
					}
					?></td>
				</tr>
				<?php
				foreach ($rl_jobs as $rl_job) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo translate('cms_report_links_future_regen_cron_info', $rl_job->id, $rl_job->readableWhen()); ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							if ($rl_job->attr('long') && $rl_job->attr('running')) {
								echo translate('cms_report_links_future_regen_running');
							}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								$user = $rl_job->getRunningUser();
								echo get_asset_tag_line($user->id);
								unset($user);
							?>
						</td>
						<td align="center" class="sq-backend-table-cell">
						<?php
							if ($rl_job->canDelete() && $asset->writeAccess('attributes')) {
								check_box($prefix.'_remove_rl[]', $rl_job->id);
							} else {
								echo '&nbsp;';
							}
						?>
						</td>
					</tr>
					<?php
				}//end foreach
			?>
			</table>
			<?php

		}//end if

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$cron_job = new Cron_Job_Regenerate_Report();
		$edit_fns = $cron_job->getEditFns();


		$o->openField(translate('cms_report_links_future_regen_field'), 'new_line');
			if ($asset->writeAccess('attributes')) {
				$cron_epoc = (int)$cron_mgr->attr('epoch');
				$cron_refresh = (int)$cron_mgr->attr('refresh_time');
				if ($cron_epoc > 0 && $cron_refresh > 0) {
					$edit_fns->paintInLineBackend($cron_job, $o, $prefix.'_new_rl');
					$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));
				} else {
					echo translate('cron_manager_not_configured_regen');
				}
			} else {
				echo translate('cms_report_links_future_regen_lock');
			}
		$o->closeField();

		return TRUE;

	}//end paintFutureRegenerate()


	/**
	* Processes the future regeneration cron job section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processFutureRegenerate(&$asset, &$o, $prefix)
	{
		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_regenerate_report');

		if (!is_null($cron_mgr)) {
			$rl_jobs = $cron_mgr->getJobs('cron_job_regenerate_report');
			$job_count = count($rl_jobs);
			for ($k = 0; $k < $job_count; $k++) {
				// only include jobs for this link report asset
				if ($rl_jobs[$k]->attr('asset') != $asset->id) {
					unset($rl_jobs[$k]);
				}
			}
			if (!empty($_POST[$prefix.'_remove_rl'])) {
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					foreach ($rl_jobs as $rl_job) {
						if (in_array($rl_job->id, $_POST[$prefix.'_remove_rl'])) {
							if ($cron_mgr->removeJob($rl_job)) {
								$is_updated = TRUE;
							}
						}// end if
					}// end for

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

				} else {
					trigger_localised_error('SYS0232', E_USER_NOTICE);
				}// end if
			}// end if

			$cron_epoc = (int)$cron_mgr->attr('epoch');
			$cron_refresh = (int)$cron_mgr->attr('refresh_time');
			if ($cron_epoc > 0 && $cron_refresh > 0) {
				if ($asset->writeAccess('attributes')) {
					$rl = new Cron_Job_Regenerate_Report();
					$edit_fns = $rl->getEditFns();

					$new_rl = $prefix.'_new_rl';
					if (isset($new_rl)) {
						if ($edit_fns->processInLineBackend($rl, $o, $new_rl)) {
							$rl->setAttrValue('long', TRUE);
							$rl->setAttrValue('asset', $asset->id);
							$rl->saveAttributes();
						} else {
							return FALSE;
						}
					} else {
						return FALSE;
					}

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					if ($cron_mgr->addJob($rl, $GLOBALS['SQ_SYSTEM']->user)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
						$is_updated = TRUE;
					} else {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				}// end if
			}//end if

		}//end if

	}//end processFutureRegenerate()


	/**
	* Paints the report broken links option
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintReportBrokenLink(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		echo hidden_field($prefix.'_report_broken', '0');

		$enabled = FALSE;
		$report_path = $asset->data_path.'/report.xml';
		$report_info = $asset->attr('report_info');
		// if report not generated, or already sent notification, then disable this button
		if (file_exists($report_path) && empty($report_info)) {
			$enabled = TRUE;
		}
		$extras = '';

		if (!$enabled) $extras = 'disabled="disabled"';

		echo normal_button($prefix.'_report_broken_button', translate('cms_report_links_broken_button'), 'this.form.'.$prefix.'_report_broken.value = 1; this.form.submit();', $extras);
		if (!$report_info) {
			$o->note(translate('cms_report_links_broken_note'));
		} else {
			$o->note($asset->attr('report_info'));
		}

		return TRUE;

	}//end paintReportBrokenLink()


	/**
	* Processes the report broken links option
	*
	* This will send emails to asset admins, reporting broken links found
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processReportBrokenLink(&$asset, &$o, $prefix)
	{
		if (!$_POST[$prefix.'_report_broken']) return TRUE;

		return $asset->reportBrokenLink();

	}//end processReportBrokenLink()


}//end class

?>
