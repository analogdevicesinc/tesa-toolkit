#-------------------------------------------------------------------------------
# Copyright (C) 2025 Analog Devices, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#-------------------------------------------------------------------------------
import os


def load_and_exec():
	# Execute JLink Script
	JLinkExe = "JLink" if os.name == "nt" else "JLinkExe"
	os.system(JLinkExe + " -device MAX32657 -if swd -speed 2000 -autoconnect 1 -CommanderScript JLinkScript")


if __name__ == '__main__':
	print("\n-------------------------")
	load_and_exec()
	print("\nImage loaded, check the PC comport")
