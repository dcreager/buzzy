require 'formula'

class Buzzy < Formula
  homepage 'https://github.com/dcreager/buzzy/'
  version '0.5.1-dev'
  url 'https://github.com/dcreager/buzzy.git', :branch => 'master'
  head 'https://github.com/dcreager/buzzy.git', :branch => 'develop'

  depends_on 'cmake' => :build
  depends_on 'check' => :build

  def install
    mkdir "build" do
      system "cmake", "..", *std_cmake_args
      system "make install"
    end
  end
end
