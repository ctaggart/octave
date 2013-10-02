## Copyright (C) 2013 Vytautas Jančauskas
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

function properties = __get_properties__ (recorder)
  properties.BitsPerSample = __recorder_get_nbits__ (struct(recorder).recorder);
  properties.CurrentSample = __recorder_get_sample_number__ (struct(recorder).recorder);
  properties.DeviceID = __recorder_get_id__ (struct(recorder).recorder);
  properties.NumberOfChannels = __recorder_get_channels__ (struct(recorder).recorder);
  if __recorder_isrecording__ (struct(recorder).recorder)
    running = "on";
  else
    running = "off";
  endif
  properties.Running = running;
  properties.SampleRate = __recorder_get_fs__ (struct (recorder).recorder);
  properties.TotalSamples = __recorder_get_total_samples__ (struct (recorder).recorder);
  properties.Tag = __recorder_get_tag__ (struct (recorder).recorder);
  properties.Type = "audiorecorder";
  properties.UserData = __recorder_get_userdata__ (struct (recorder).recorder);
endfunction
