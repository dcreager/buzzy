require 'formula'

class Buzzy < Formula
  homepage 'https://github.com/redjack/buzzy/'
  version '0.3.1'
  url 'https://github.com/redjack/buzzy.git', :branch => 'master'
  head 'https://github.com/redjack/buzzy.git', :branch => 'develop'

  depends_on 'cmake' => :build
  depends_on 'check' => :build

  def install
    mkdir "build" do
      system "cmake", "..", *std_cmake_args
      system "make install"
    end
  end
end
