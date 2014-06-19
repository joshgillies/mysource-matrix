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
* $Id: data_record_edit_fns.inc,v 1.4 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Data_Record_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_View_Edit_Fns extends Asset_Edit_Fns
{

    /**
     * Not set text from GA.
     *
     * @const string.
     */
    const NOT_SET = '(not set)';

    /**
     * Not provided text from GA.
     *
     * @const string.
     */
    const NOT_PROVIDED = '(not provided)';
    

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		
		// enable just the details
		// disable lock section
		$this->static_screens = Array();
		$this->static_screens['details']['name'] = translate('Details');
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'none';

	}//end constructor
	
	
	/**
	 * Paint the "overall" report.
	 *
	 */
	public function paintSummaryOverallReport($asset, &$o, $prefix)
	{
	    $months = 1;
	    $start_date = strtotime('-'.$months.' months 00:00:00');
	    $end_date   = strtotime('yesterday 00:00:00');
	    $day_count  = round(1 + (($end_date - $start_date) / 86400), 0);
	    $report = $asset->generateReport(
	        Array(),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $report = array_get_index($report, 0, Array());
	    
	    // Give us some goal stats for this period, five at a time (because
	    // 10 metrics maximum).
	    $goals = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->bridgeid)->getGoals($asset->viewid);
	    $goals_reports = Array();
	    foreach (array_chunk(array_keys($goals), 5) as $goalids) {
	        $goal_metrics = Array();
	        foreach ($goalids as $goalid) {
	            $goal_metrics[] = 'ga:goal'.$goalid.'Starts';
	            $goal_metrics[] = 'ga:goal'.$goalid.'Completions';
	        }
	        $goals_report = $asset->generateReport(
                Array(),
                $goal_metrics,
                $start_date,
                $end_date
	        );
	        $goals_reports = array_merge($goals_reports, $goals_report[0]);
	    }//end foreach
	    
	    // But we still need to calculate our own rate, because Starts only
	    // has meaning for a funnel. Anything that doesn't have a funnel will
	    // have the same number of starts as completions, so reset that to
	    // the number of sessions.
	    foreach ($goals as $goal) {
	        if (($goal['type'] !== 'URL_DESTINATION') || ($goal['urlDestinationDetails']['firstStepRequired'] === FALSE)) {
	            $report['ga:goalStartsAll'] -= $goals_reports['ga:goal'.$goal['id'].'Starts'];
	            $report['ga:goalStartsAll'] += $report['ga:sessions'];
	            
	            $goals_reports['ga:goal'.$goal['id'].'Starts'] = NULL;
	            if ($report['ga:sessions'] === 0) {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = NULL;
	            } else {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = $goals_reports['ga:goal'.$goal['id'].'Completions'] / $report['ga:sessions'];
	            }
	        } else {
	            if ($goals_reports['ga:goal'.$goal['id'].'Starts'] === 0) {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = NULL;
	            } else {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = $goals_reports['ga:goal'.$goal['id'].'Completions'] / $goals_reports['ga:goal'.$goal['id'].'Starts'];
	            }
	        }
	    }//end foreach
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    // Sessions and unique users.
	    $o->openSection(translate('Sessions and users'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total sessions') ?></th>
	            <td><strong><?php echo number_format($report['ga:sessions'], 0); ?></strong></td>
	            <td><?php echo translate('%s / day', number_format($report['ga:sessions'] / $day_count, 0)); ?></td>
	            <td><?php echo translate(
	                '%s (%s%%) bounces',
	                number_format($report['ga:bounces'], 0),
	                number_format((100.0 * $report['ga:bounces'] / $report['ga:sessions']), 1)
                ); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Unique users') ?></th>
	            <td><strong><?php echo number_format($report['ga:users'], 0); ?></strong></td>
	            <td colspan="2"><?php echo translate('%s / day', number_format($report['ga:users'] / $day_count, 0)); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('New users') ?></th>
	            <td colspan="3"><?php echo number_format($report['ga:newUsers'], 0); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Sessions per user') ?></th>
	            <td colspan="3"><?php echo number_format(($report['ga:sessions'] / $report['ga:users']), 2); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Return visits per user') ?></th>
	            <td><?php echo number_format(($report['ga:sessions'] / ($report['ga:users'] - $report['ga:newUsers']) - 1), 2); ?></td>
	            <td colspan="2"><?php echo translate('Excludes first visit from a new user during this period'); ?></td>
	        </tr>
	    </tbody></table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    // Session statistics.
	    $o->openSection(translate('Session statistics'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total page views') ?></th>
	            <td><strong><?php echo number_format($report['ga:pageviews'], 0); ?></strong></td>
	            <td colspan="2"><?php echo translate('%s / day', number_format($report['ga:pageviews'] / $day_count, 0)); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Page views per session') ?></th>
	            <td><?php echo number_format($report['ga:pageviews'] / $report['ga:sessions'], 1); ?></td>
	            <td colspan="2"><?php echo translate(
	                '%s excluding bounces',
	                number_format($report['ga:pageviews'] / ($report['ga:sessions'] - $report['ga:bounces']), 1)
	            ); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Session duration') ?></th>
	            <td><?php 
	            $time = $report['ga:sessionDuration'] / $report['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), floor($time % 60));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), (floor($time * 10) / 10));
                    }
	            ?></td>
	            <td colspan="2"><?php
	                $nb_time = $report['ga:sessionDuration'] / ($report['ga:sessions'] - $report['ga:bounces']);
                    if ($nb_time >= 60) {
                        $nb_str = sprintf(translate('%s min %s sec', '%s', '%s'), (int)($nb_time / 60), floor($nb_time % 60));
                    } else {
                        $nb_str = sprintf(translate('%s sec', '%s'), (floor($nb_time * 10) / 10));
                    }
                        
                    echo translate('%s excluding bounces', $nb_str);
	            ?></td>
	        </tr>
	    </tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    // Start the section on goal stats.
	    $o->openSection(translate('Goal statistics'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="30%" />
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <thead>
	        <tr>
	            <th><?php echo translate('Goal name') ?></th>
	            <th><?php echo translate('Goal type') ?></th>
	            <th><?php echo translate('Completion ratio') ?></th>
	            <th><?php echo translate('Completions') ?></th>
	            <th><?php echo translate('Starts') ?></th>
	        </tr>
	    <tbody><?php
	        if (count($goals) === 0) {
	            ?><tr>
	            <td colspan="5"><?php echo translate('No goals have been created for this view'); ?></td>
	            </tr><?php
	        } else {
                ?><tr>
                    <th><?php echo translate('All goals') ?></th>
                    <td></td>
                    <td><strong><?php
                        if ($report['ga:goalStartsAll'] > 0) {
                            echo number_format($report['ga:goalCompletionsAll'] / $report['ga:goalStartsAll'] * 100.0, 1).'%';
                        } else {
                            echo translate('N/A');
                        }//end if
                    ?></strong></td>
                    <td><strong><?php echo number_format($report['ga:goalCompletionsAll'], 0); ?></strong></td>
                    <td><strong><?php echo number_format($report['ga:goalStartsAll'], 0); ?></strong></td>	           
                </tr><?php
                foreach ($goals as $goal) {
                ?><tr>
                    <th><?php echo $goal['id'].'. '.$goal['name'] ?></th>
                    <td><?php
                        switch ($goal['type']) {
                            case 'URL_DESTINATION':
                                if ($goal['urlDestinationDetails']['firstStepRequired'] === TRUE) {
                                    echo translate('URL (Funnel)');
                                } else {
                                    echo translate('URL (Single Page)');
                                }
                            break;
                            case 'EVENT':
                                echo translate('Event Tracking');
                            break;
                            case 'VISIT_NUM_PAGES':
                                echo translate('Page Views');
                            break;
                            case 'VISIT_TIME_ON_SITE':
                                echo translate('Session Duration');
                            break;
                            default:
                                echo translate('Unknown');
                            break;
                        }
                    ?></td>
                    <td><strong><?php
                        if ($goals_reports['sq:goal'.$goal['id'].'Rate'] !== NULL) {
                            echo number_format($goals_reports['sq:goal'.$goal['id'].'Rate'] * 100.0, 1).'%';
                        } else {
                            echo translate('N/A');
                        }//end if
                    ?></strong></td>
                    <td><?php echo number_format($goals_reports['ga:goal'.$goal['id'].'Completions'], 0); ?></td>
                    <td><?php echo number_format(array_get_index($goals_reports, 'ga:goal'.$goal['id'].'Starts', $report['ga:sessions']), 0); ?></td>	           
                </tr><?php
                }//end foreach
            }//end if
	        ?></tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    // Start the section on experiment stats (holy bleep).
	    $experiments = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->bridgeid)->getExperiments($asset->viewid);
	    $o->openSection(translate('Content Experiments'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="30%" />
	    <col width="15%" />
	    <col width="23%" />
	    <col width="8%" />
	    <col width="8%" />
	    <col width="8%" />
	    <col width="8%" />
	    <thead>
	        <tr>
	            <th><?php echo translate('Experiment name and description') ?></th>
	            <th><?php echo translate('Status') ?></th>
	            <th><?php echo translate('Variations') ?></th>
	            <th><?php echo translate('Traffic Coverage') ?></th>
	            <th><?php echo translate('Variation Weighting') ?></th>
	            <th><?php echo translate('Minimum Length') ?></th>
	            <th><?php echo translate('Winner Confidence') ?></th>
	        </tr>
	    <tbody><?php
	        if (count($experiments) === 0) {
	            ?><tr>
	            <td colspan="7"><?php echo translate('No content experiments available for this view'); ?></td>
	            </tr><?php
	        } else {	       
                foreach ($experiments as $experiment) {
                ?><tr>
                    <th style="font-weight: normal"><strong><?php echo $experiment['name'] ?></strong><br/>
                    Target metric: <strong><?php echo $experiment['objectiveMetric'] ?></strong><br/>
                    <?php echo ellipsisize($experiment['description'], 150); ?></th>
                    <td><?php
                        switch ($experiment['status']) {
                            case 'DRAFT':
                                echo translate('Draft');
                            break;
                            case 'READY_TO_RUN':
                                echo translate('Ready to run');
                            break;
                            case 'RUNNING':
                                echo translate('Running');
                            break;
                            case 'ENDED':
                                $end_time = strtotime($experiment['endTime']);
                                if (strtotime('+2 days', $end_time) > time()) {
                                    echo translate('Just ended');
                                } else {
                                    echo translate('Ended %s', date('jS M Y', $end_time));
                                }
                                
                                echo '<br/>';
                                switch ($experiment['reasonExperimentEnded']) {
                                    case 'WINNER_FOUND':
                                        foreach (array_get_index($experiment, 'variations', Array()) as $variation) {
                                            if ($variation['won']) {
                                                echo translate('<strong>%s</strong> won', $variation['name']);
                                                break;
                                            }
                                        }
                                    break;
                                    case 'EXPERIMENT_EXPIRED':
                                        echo translate('Expired before winner found');
                                    break;
                                    case 'ENDED_WITH_NO_WINNER':
                                        echo translate('Ended without a winner');
                                    break;
                                    case 'STOPPED_BY_USER':
                                        echo translate('Stopped by user');
                                    break;
                                    default:
                                        // Do nothing.
                                    break;
                                }
                            break;
                            default:
                                echo translate('Unknown');
                            break;
                        }
                    ?></td>
                    <td><?php
                    $variation_list = Array();
                    foreach (array_get_index($experiment, 'variations', Array()) as $variation) {
                        $variation_str = $variation['name'];
                        if (array_get_index($variation, 'inactive', FALSE) === TRUE) {
                            $variation_str .= ' ('.translate('Inactive').')';
                        }
                        if (array_key_exists('weight', $variation) === TRUE) {
                            $variation_str .= ' ('.number_format($variation['weight'] * 100.0, 0).'%)';
                        }
                        $variation_list[] = $variation_str;
                    }
                    echo '<ol><li>'.implode('</li><li>', $variation_list).'</li></ol>';
                    ?></td>
                    <td><?php echo translate('%s%%', $experiment['trafficCoverage'] * 100.0) ?></td>
                    <td><?php 
                    if ($experiment['equalWeighting'] === TRUE) {
                        echo translate('Equal');
                    } else {
                        echo translate('Dynamic');
                    } ?></td>
                    <td><?php echo translate('%s days', $experiment['minimumExperimentLengthInDays']) ?></td>
                    <td><?php echo translate('%s%%', number_format($experiment['winnerConfidenceLevel'] * 100.0, 1)) ?></td>
                </tr><?php
                }//end foreach
            }//end if
	        ?></tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	    return FALSE;
	    
	}//end paintSummaryOverallReport()
	
	/**
	 * Paint the geographic reports.
	 *
	 */
	public function paintGeoCountryReport($asset, &$o, $prefix)
	{
	    $max_results = 20;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:country',
	            'ga:region',
	            'ga:city',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date,
	        Array('-ga:sessions')
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $city_report    = Array();
	    
	    $i = 0;
	    foreach ($raw_report as $rowid => &$row) {
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:country'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $country = $row['ga:country'];
	            $city    = $row['ga:city'];
	            
	            if ($city === self::NOT_SET) {
	                $city = $row['ga:region'];
	            } else if ($row['ga:region'] !== self::NOT_SET) {
	                $city .= ', '.$row['ga:region'];
	            }
	            
	            if ($city !== self::NOT_SET) {
	                $i++;
	                $city_report[$country] = array_get_index($city_report, $country, Array());
                    $city_report[$country][$city] = $row;
                    $city_report[$country][$city]['sq:cityRegion'] = $city;
                    $city_report[$country][$city]['sq:rank'] = $i;
                }
	            
	            if (array_key_exists($country, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (is_numeric($value) === TRUE) {
	                        $report[$country][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$country] = $row;
	            }
	        }
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($city_report as $country => &$city_rows) {
            usort($city_rows, function($a, $b) {
                return $a['sq:rank'] - $b['sq:rank'];
            });
            $city_rows = array_slice($city_rows, 0, 5);
	    }	  
	    
	    $other_report = array_reduce(
	        array_slice($report, $max_results),
	        function($carry, $row) {
	            if (empty($carry) === FALSE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (is_numeric($value) === TRUE) {
	                        $carry[$fieldid] += $value;
	                    }
	                }
	            } else {
	                $carry = $row;
	            }
	            
	            return $carry;
	        },
	        Array()
	    );
	    
	    $other_report['ga:country'] = translate('Other countries (%s)', count($report) - $max_results);
	    $other_report['sq:other']   = TRUE;
	    $report = array_slice($report, 0, $max_results);
	    $report[] = $other_report;
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provide location information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Country') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $country = $row['ga:country'];
                if (array_key_exists('sq:other', $row) === FALSE) {
                    echo $rank.'. '.$row['ga:country'];
                } else {
                    echo $row['ga:country'];
                } ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                if (array_key_exists('sq:other', $row) === FALSE) {
                    foreach ($city_report[$country] as $city_row) {
                    ?><tr>
                        <td><?php 
                        echo '<span style="float: left; padding-left: 3ex">';
                        echo $city_row['sq:cityRegion'];
                        echo '</span><span style="float: right"><em>(#'.$city_row['sq:rank'].')</em></span>'; 
                        ?></td>
                        <td style="text-align: right"><strong><?php echo number_format($city_row['ga:sessions'], 0); ?></strong></td>
                        <td style="text-align: right"><?php echo number_format($city_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                        <td style="text-align: right"><?php echo number_format($city_row['ga:users'], 0); ?></td>
                        <td style="text-align: right"><?php
                        if ($city_row['ga:sessions'] === 0) {
                            echo translate('N/A');
                        } else {
                            echo number_format($city_row['ga:pageviews'] / $city_row['ga:sessions'], 1);
                        }
                        ?></td>
                        <td style="text-align: right"><?php
                        if ($city_row['ga:sessions'] === 0) {
                            echo translate('N/A');
                        } else {
                            $time = $city_row['ga:sessionDuration'] / $city_row['ga:sessions'];
                            if ($time >= 60) {
                                echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                            } else {
                                echo sprintf(translate('%s sec', '%s'), floor($time));
                            }
                        }
                        ?></td>
                    </tr><?php
                    }//end if city rows
                }//end if not other
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintGeographicReport()
	
	
	/**
	 * Paint the geographic reports.
	 *
	 */
	public function paintGeoCityReport($asset, &$o, $prefix)
	{
	    $months = 1;
	    $start_date = strtotime('-'.$months.' months 00:00:00');
	    $end_date   = strtotime('yesterday 00:00:00');
	    $day_count  = round(1 + (($end_date - $start_date) / 86400), 0);
	    $report = $asset->generateReport(
	        Array(
	            'ga:country',
	            'ga:region',
	            'ga:city',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date,
	        Array('-ga:sessions')
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    foreach ($report as $rowid => &$row) {
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:city'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        }
	    }//end foreach
	    
	    $report = array_slice($report, 0, 50);
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
        
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provide a city',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="20%" />
	    <col width="20%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('City Name') ?></th>
	            <th><?php echo translate('Country') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php echo $rank.'. '.$row['ga:city']; ?></th>
                <td><?php echo $row['ga:country']; ?></strong></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></td>
                <td style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                <td style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></td>
                <td style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></td>
                <td style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></td>
            </tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintGeoCityReport()
	
	
//--        TECHNOLOGY REPORTS        --//



	
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesOSesReportDesktop($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'desktop') {
	            continue;
	        }
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:operatingSystem'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:operatingSystem'];
	            $version = $row['ga:operatingSystemVersion'];
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided operating system information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
	            $translated_row = $this->_translateDesktopOS(
                    $row['ga:operatingSystem'],
                    self::NOT_SET
                );
            ?><tr>
                <th><?php 
                $os = $row['ga:operatingSystem'];
                echo $rank.'. '.$translated_row['ga:operatingSystem'];
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                $translated_row = $this->_translateDesktopOS(
                    $version_row['ga:operatingSystem'],
                    $version_row['ga:operatingSystemVersion']
                );
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $translated_row['ga:operatingSystemVersion'].'</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportDesktop()



	
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesOSesReportTablet($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'tablet') {
	            continue;
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateMobileOS(
	                $row['ga:operatingSystem'],
	                $row['ga:operatingSystemVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:operatingSystem'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:operatingSystem'];
	            $version = $row['ga:operatingSystemVersion'];
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided operating system information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['ga:operatingSystem'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:operatingSystemVersion'].'</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportTablet()

	
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesOSesReportMobile($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'mobile') {
	            continue;
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateMobileOS(
	                $row['ga:operatingSystem'],
	                $row['ga:operatingSystemVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:operatingSystem'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:operatingSystem'];
	            $version = $row['ga:operatingSystemVersion'];
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided operating system information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['ga:operatingSystem'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:operatingSystemVersion'].'</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportMobile()
	
	
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesDevicesReportTablet($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:mobileDeviceBranding',
	            'ga:mobileDeviceModel',
	            'ga:mobileDeviceMarketingName',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'tablet') {
	            continue;
	        }
	        
	        $row['sq:mobileName'] = $row['ga:mobileDeviceBranding'];
	        if ($row['ga:mobileDeviceMarketingName'] === self::NOT_SET) {
	            $row['sq:mobileName'] .= ' '.$row['ga:mobileDeviceModel'];
	        } else {
	            $row['sq:mobileName'] .= ' '.$row['ga:mobileDeviceMarketingName'];
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateMobileOS(
	                $row['ga:operatingSystem'],
	                $row['ga:operatingSystemVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:mobileDeviceBranding'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['sq:mobileName'];
	            $version = $row['ga:operatingSystem'].' ('.$row['ga:operatingSystemVersion'].')';
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided mobile device branding information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['sq:mobileName'];
                echo '<span style="float: left">'.$rank.'. '.$os.'</span>';
                if (count($version_report[$os]) === 1) {
                    echo '<span style="float: right; font-weight: normal"><em>';
                    echo $version_report[$os][0]['ga:operatingSystem'];
                    if ($version_report[$os][0]['ga:operatingSystemVersion'] !== self::NOT_SET) {
                        echo ' ('.$version_report[$os][0]['ga:operatingSystemVersion'].')';
                    }
                    echo '</em></span>';
                    $version_report[$os] = Array();
                }
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:operatingSystem'].' ('.$version_row['ga:operatingSystemVersion'].')</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportMobile()
	
	
	
	
	
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesDevicesReportMobile($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:mobileDeviceBranding',
	            'ga:mobileDeviceModel',
	            'ga:mobileDeviceMarketingName',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'mobile') {
	            continue;
	        }
	        
	        $row['sq:mobileName'] = $row['ga:mobileDeviceBranding'];
	        if ($row['ga:mobileDeviceMarketingName'] === self::NOT_SET) {
	            $row['sq:mobileName'] .= ' '.$row['ga:mobileDeviceModel'];
	        } else {
	            $row['sq:mobileName'] .= ' '.$row['ga:mobileDeviceMarketingName'];
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateMobileOS(
	                $row['ga:operatingSystem'],
	                $row['ga:operatingSystemVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:mobileDeviceBranding'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['sq:mobileName'];
	            $version = $row['ga:operatingSystem'].' ('.$row['ga:operatingSystemVersion'].')';
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided mobile device branding information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['sq:mobileName'];
                echo '<span style="float: left">'.$rank.'. '.$os.'</span>';
                if (count($version_report[$os]) === 1) {
                    echo '<span style="float: right; font-weight: normal"><em>';
                    echo $version_report[$os][0]['ga:operatingSystem'];
                    if ($version_report[$os][0]['ga:operatingSystemVersion'] !== self::NOT_SET) {
                        echo ' ('.$version_report[$os][0]['ga:operatingSystemVersion'].')';
                    }
                    echo '</em></span>';
                    $version_report[$os] = Array();
                }
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:operatingSystem'].' ('.$version_row['ga:operatingSystemVersion'].')</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportTablet()
	
	

	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesBrowsersReportDesktop($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'desktop') {
	            continue;
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateDesktopBrowser(
	                $row['ga:browser'],
	                $row['ga:browserVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:browser'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:browser'];
	            $version = $row['ga:browserVersion'];
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if ((($fieldid !== 'ga:operatingSystemVersion') && ($fieldid !== 'ga:browserVersion')) && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if ((($fieldid !== 'ga:operatingSystemVersion') && ($fieldid !== 'ga:browserVersion')) && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided browser information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['ga:browser'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:browserVersion'].'</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportMobile()
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesBrowsersReportTablet($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'tablet') {
	            continue;
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateMobileBrowser(
	                $row['ga:browser'],
	                $row['ga:browserVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:browser'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:browser'];
	            $version = $row['ga:browserVersion'];
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided browser information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['ga:browser'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:browserVersion'].'</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportMobile()
	
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesBrowsersReportMobile($asset, &$o, $prefix)
	{
	    $max_results = 100;
	    $months      = 1;
	    $start_date  = strtotime('-'.$months.' months 00:00:00');
	    $end_date    = strtotime('yesterday 00:00:00');
	    $day_count   = round(1 + (($end_date - $start_date) / 86400), 0);
	    
	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date,
	        $end_date
	    );
	    
	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();
	    
	    foreach ($raw_report as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'mobile') {
	            continue;
	        }
	        
	        $row = array_merge(
	            $row,
	            $this->_translateMobileBrowser(
	                $row['ga:browser'],
	                $row['ga:browserVersion']
	            )
	        );
	        
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:browser'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:browser'];
	            $version = $row['ga:browserVersion'];
	            
	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if
	            
	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach
	    
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });
	    
	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );
	    
            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided browser information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();
	    
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php 
                $os = $row['ga:browser'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php 
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:browserVersion'].'</span>'; 
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();
	    
	    return FALSE;
	    
	}//end paintTechnologiesOSesReportMobile()
	
	
	private function _translateDesktopOS($os, $version) {
	    switch ($os) {
    	    case 'Windows':
    	        $version = 'Windows '.$version;
	        break;
	        
		    case 'Macintosh':
		        $os = 'OS X (Mac)';
		        
		        // Remove the architecture.
		        list(, $version) = explode(' ', $version);
		        
		        $version_list = Array(
		            '10.10' => 'Yosemite',
		            '10.9'  => 'Mavericks',
		            '10.8'  => 'Mountain Lion',
		            '10.7'  => 'Lion',
		            '10.6'  => 'Snow Leopard',
		            '10.5'  => 'Leopard',
		            '10.4'  => 'Tiger',
		            '10.3'  => 'Panther',
		        );
		        
		        if (array_key_exists($version, $version_list) === TRUE) {
		            $version = $version_list[$version].' ('.$version.')';
		        }
	        break;
	        
		    case 'Linux':
		        if (($version === 'x86_64') || ($version === 'amd64')) {
		            $version = translate('64-bit (x86-64)');
		        } else if ($version === 'ia64') {
		            $version = translate('64-bit (Itanium)');
		        } else if (preg_match('/^i[3-6]86$/', $version) > 0) {
		            $version = translate('32-bit');
		        } else {
		            $version = self::NOT_SET;
		        }
	        break;
	    }//end switch
	    
	    return Array(
	        'ga:operatingSystem'        => $os,
	        'ga:operatingSystemVersion' => $version,
	    );
	    
	}//end translateDesktopOSName()
	
	
	private function _translateMobileOS($os, $version) {
	    switch ($os) {
		    case 'iOS':
		        if ($version === self::NOT_SET) {
		            $os = translate('iOS (unknown version)');
		        } else {
		            $version_parts = explode('.', $version);
		            $os = 'iOS '.$version_parts[0];
		        }
	        break;
	        
		    case 'Android':
		        $version_list = Array(
		            '4\.4'     => 'KitKat',
		            '4\.[123]' => 'Jelly Bean',
		            '4\.0'     => 'Ice Cream Sandwich',
		            '3\.'      => 'Honeycomb',
		            '2\.3'     => 'Gingerbread',
		            '2\.2'     => 'Froyo',
		            '2\.[01]'  => 'Eclair',
		            '1\.6'     => 'Donut',
		            '1\.5'     => 'Cupcake',
		        );
		        
		        if ($version === self::NOT_SET) {
		            $os = translate('Android (unknown version)');
		        } else {
                    $os = translate('Android 1.1 or lower');
                    foreach ($version_list as $regex => $version_name) {
                        if (preg_match('/^'.$regex.'(\.[\d.]+)?$/', $version) > 0) {
                            $os = 'Android '.$version_name;
                            break;
                        }
                    }
		        }
	        break;
	    }//end switch
	    
	    return Array(
	        'ga:operatingSystem'        => $os,
	        'ga:operatingSystemVersion' => $version,
	    );
	    
	}//end translateDesktopOSName()
	
	
	private function _translateDesktopBrowser($browser, $version) {
	    switch ($browser) {
	        case 'Chrome':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	            
	            $version_list = Array(
	                '32.0' => translate('2014 (versions 32+)'),
	                '28.0' => translate('2013, Blink (versions 28-31)'),
	                '24.0' => translate('2013, WebKit (versions 24-27)'),
	                '17.0' => translate('2012 (versions 17-23)'),
	                '9.0'  => translate('2011 (versions 9-16)'),
	                '0.0'  => translate('2010 or earlier (versions 0.x-8)'),
	            );
	            
	            foreach ($version_list as $version_minimum => $version_str) {
	                if (version_compare($version, $version_minimum, '>=') === TRUE) {
	                    $version = $version_str;
	                    break;
	                }
	            }
	        break;
	        
	        case 'Firefox':
                $version_parts = explode('.', $version);
                if ((int) $version_parts[0] >= 4) {
                    $esr_version = floor(($version_parts[0] - 3) / 7) * 7 + 3;
                    if ($esr_version < 10) {
                        // Pre-ESR. Group rapid releases of 4.x+ together.
                        $esr_version      = 4;
                        $esr_last_version = 9;
                    } else {
                        // By rule, ESR versions currently span 7 rapid relases.
                        $esr_last_version = $esr_version + 6;
                    }
                    $version = $esr_version.'-'.$esr_last_version;
	            } else {
                    $version = $version_parts[0].'.'.$version_parts[1];
                }
	        break;
	        
	        case 'Safari':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;
	        
	        case 'Opera':
	            $version_parts = explode('.', $version);
	            
	            if ($version_parts[0] >= 15) {
	                // Blink-based Opera.
                    $version_list = Array(
                        '19.0'  => translate('Blink, 2014 (versions 19+)'),
                        '15.0'  => translate('Blink, 2013 (versions 15-18)'),
                    );
	            
                    foreach ($version_list as $version_minimum => $version_str) {
                        if (version_compare($version, $version_minimum, '>=') === TRUE) {
                            $version = $version_str;
                            break;
                        }
                    }
	            } else {
	                // Classic Opera. Format it as, eg. '12.5x'
	                $version = translate('Presto (up to version 12)'); 
	                //$version = $version_parts[0].'.'.floor($version_parts[1] / 10).'x';
	            }
	        break;
	        
	        case 'Internet Explorer':
	            if ($version === '999.1') {
	                // Certain anti-viruses set IE's version to a dodgy value
	                // to stop people gleaning security issues from their
	                // version number. Not helpful.
	                $version = self::NOT_SET;
	            }
	    }
	    
	    return Array(
	        'ga:browser'        => $browser,
	        'ga:browserVersion' => $version,
	    );
	    
	    return Array(
	        'ga:browser'        => $browser,
	        'ga:browserVersion' => $version,
	    );
	    
	}//end translateDesktopOSName()
	
	
	private function _translateMobileBrowser($browser, $version) {
	    switch ($browser) {
	        case 'Chrome':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;
	        
	        case 'Firefox':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;
	        
	        case 'Safari':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;
	        
	        case 'Opera Mini':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.'.$version_parts[1];
	        break;
	    }
	    
	    return Array(
	        'ga:browser'        => $browser,
	        'ga:browserVersion' => $version,
	    );
	    
	}//end translateDesktopOSName()

}//end class

?>
